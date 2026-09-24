#!/usr/bin/env python3
"""Pool terminal FH_TELEM records without treating missing arms as zero work."""
import argparse
import json
from pathlib import Path
import sys

SCALARS = ('worker_ns', 'cell_ns', 'score_ns_est', 'sort_ns', 'complete_ns',
           'score_calls', 'score_samples', 'completions', 'hist_candidates',
           'hist_nodes', 'total_nodes', 'unranked_nodes', 'unranked_count',
           'unranked_ns', 'cells_live_done', 'cells_partial', 'resume_replay_ns',
           'resume_replay_score_ns_est', 'resume_replay_sort_ns', 'resume_cells')
ARRAYS = ('depth_nodes', 'rank_nodes', 'rank_count', 'rank_ns', 'rank_aborts')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def nonnegative_int(value):
    return type(value) is int and value >= 0


def validate(r):
    require(isinstance(r, dict), 'record must be an object')
    for key in ('version', 'n', 'stride') + SCALARS:
        require(nonnegative_int(r.get(key)), f'{key} must be a nonnegative integer')
    require(r['version'] == 2, 'unsupported version')
    require(r['n'] > 0, 'n must be positive')
    require(r['stride'] in (1, 64), 'stride must be 1 or 64')
    for key in ARRAYS:
        length = (r['n'] + 1)//2 + 1 if key == 'depth_nodes' else 10
        values = r.get(key)
        require(isinstance(values, list) and len(values) == length and
                all(nonnegative_int(v) for v in values), f'invalid {key} array')
    require(sum(r['depth_nodes']) == r['hist_nodes'], 'depth_nodes != hist_nodes')
    require(sum(r['rank_nodes']) + r['unranked_nodes'] == r['total_nodes'],
            'rank nodes != total_nodes')
    require(sum(r['rank_count']) + r['unranked_count'] == r['completions'],
            'rank counts != completions')
    require(sum(r['rank_ns']) + r['unranked_ns'] == r['complete_ns'],
            'rank timing != complete_ns')
    require(all(a <= c for a, c in zip(r['rank_aborts'], r['rank_count'])),
            'rank aborts exceed counts')
    require(r['hist_nodes'] <= r['total_nodes'], 'hist_nodes exceeds total_nodes')
    require(r['hist_candidates'] <= r['completions'], 'hist_candidates exceeds completions')
    require(r['score_samples'] <= r['score_calls'], 'score_samples exceeds score_calls')
    if r['stride'] == 1:
        require(r['hist_nodes'] == r['total_nodes'], 'full histogram nodes incomplete')
        require(r['hist_candidates'] == r['completions'], 'full histogram candidates incomplete')
        require(r['score_samples'] == r['score_calls'], 'full scoring samples incomplete')
    require(r['worker_ns'] >= r['cell_ns'], 'cell_ns exceeds worker_ns')
    require(r['cell_ns'] >= r['sort_ns'] + r['complete_ns'],
            'sort_ns + complete_ns exceeds cell_ns')
    replay = r['resume_replay_ns']
    replay_sort = r['resume_replay_sort_ns']
    replay_score = r['resume_replay_score_ns_est']
    gross = r['cell_ns'] - r['sort_ns'] - r['complete_ns']
    require(replay_sort <= r['sort_ns'], 'replay sort exceeds total sort')
    require(replay_sort <= replay, 'replay sort exceeds replay interval')
    require(replay_score <= r['score_ns_est'], 'replay score exceeds total score')
    require(replay - replay_sort <= gross, 'replay exceeds non-completion interval')
    if r['stride'] == 1:
        require(r['score_ns_est'] <= gross, 'full scoring exceeds stream interval')
        require(replay_score <= replay - replay_sort,
                'full replay scoring exceeds replay stream interval')
        require(gross - replay + replay_sort - r['score_ns_est'] + replay_score >= 0,
                'full scoring exceeds non-replay stream interval')


def last_record(path):
    last = None
    with path.open() as log:
        for line in log:
            if line.startswith('FH_TELEM '):
                last = line[len('FH_TELEM '):]
    if last is None:
        return None
    record = json.loads(last)
    validate(record)
    return record


def ratio(numerator, denominator):
    return numerator / denominator if denominator else None


def aggregate(paths, expected):
    require(expected > 0, 'expected-arms must be positive')
    require(len(paths) <= expected, 'more input paths than expected arms')
    resolved = [p.resolve() for p in paths]
    require(len(set(resolved)) == len(resolved), 'duplicate input paths')
    records, missing, rejected = [], [], []
    for path in paths:
        try:
            r = last_record(path)
        except (OSError, ValueError, TypeError) as exc:
            missing.append(str(path))
            rejected.append(dict(path=str(path), reason=str(exc)))
            continue
        if r is None:
            missing.append(str(path))
        else:
            records.append(r)
    configs = {(r['version'], r['n'], r['stride']) for r in records}
    require(len(configs) <= 1, 'mixed version/n/stride configurations')
    version, n, stride = next(iter(configs)) if configs else (None, None, None)
    s = dict(version=version, n=n, stride=stride, expected_arms=expected,
             arms_reported=len(records), arms_missing=expected-len(records),
             missing_input_arms=expected-len(paths), missing_record_paths=missing,
             complete_coverage=len(records) == expected,
             coverage_acceptable=bool(records) and expected-len(records) <= 3,
             rejected_records=rejected,
             missing_arms_bias_warning=(
                 'Missing or rejected arms may be systematically different; '
                 'reported totals are not scaled and losses must not be assumed random.'
                 if len(records) != expected else None))
    for key in SCALARS:
        s[key] = sum(r[key] for r in records)
    for key in ARRAYS:
        s[key] = [sum(values) for values in zip(*(r[key] for r in records))]
    s['stream_gross_ns'] = s['cell_ns'] - s['sort_ns'] - s['complete_ns']
    s['stream_other_ns_est'] = s['stream_gross_ns'] - s['score_ns_est']
    s['decomposition_valid'] = s['stream_other_ns_est'] >= 0
    s['other_ns'] = s['worker_ns'] - s['cell_ns']
    for key, numerator in [('g', s['cell_ns'] - s['complete_ns']),
                           ('scoring_share', s['score_ns_est']),
                           ('sort_share', s['sort_ns']),
                           ('completion_share', s['complete_ns']),
                           ('other_share', s['other_ns'])]:
        s[key] = ratio(numerator, s['worker_ns'])
    for key, replay_key in [('worker_ns', 'resume_replay_ns'),
                             ('cell_ns', 'resume_replay_ns'),
                             ('score_ns_est', 'resume_replay_score_ns_est'),
                             ('sort_ns', 'resume_replay_sort_ns')]:
        s[key + '_without_replay'] = s[key] - s[replay_key]
    s['stream_other_ns_est_without_replay'] = (
        s['cell_ns_without_replay'] - s['sort_ns_without_replay'] -
        s['complete_ns'] - s['score_ns_est_without_replay'])
    s['decomposition_without_replay_valid'] = s['stream_other_ns_est_without_replay'] >= 0
    for key, numerator in [('g', s['cell_ns_without_replay'] - s['complete_ns']),
                           ('scoring_share', s['score_ns_est_without_replay']),
                           ('sort_share', s['sort_ns_without_replay']),
                           ('completion_share', s['complete_ns']),
                           ('other_share', s['other_ns'])]:
        s[key + '_without_replay'] = ratio(numerator, s['worker_ns_without_replay'])
    s['replay_share'] = ratio(s['resume_replay_ns'], s['worker_ns'])
    half = (n + 1)//2 if n is not None else 0
    s['late_nodes'] = sum(nodes for d, nodes in enumerate(s['depth_nodes'])
                          if 4 * max(0, half-d-1) < n)
    s['late_share'] = ratio(s['late_nodes'], s['hist_nodes'])
    s['histogram_mode'] = 'sampled' if stride == 64 else 'full' if stride == 1 else None
    s['stats_sufficient'] = (s['cells_live_done'] >= 500 and s['completions'] >= 25000
                             and s['hist_nodes'] >= 1000000000)
    s['stats_criterion'] = ('sampled insufficient' if stride == 64 and
                            not s['stats_sufficient'] else 'full mode criterion')
    s['pilot_read_usable'] = (s['coverage_acceptable'] and s['stats_sufficient'] and
                              s['decomposition_valid'] and s['decomposition_without_replay_valid'])
    for flag, share, threshold in [('g_ge_30', 'g', .30),
                                   ('complete_ge_70', 'completion_share', .70),
                                   ('late_ge_25', 'late_share', .25),
                                   ('g_ge_30_without_replay', 'g_without_replay', .30),
                                   ('complete_ge_70_without_replay',
                                    'completion_share_without_replay', .70)]:
        s[flag] = s[share] >= threshold if s[share] is not None else None
    return s


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--expected-arms', type=int, required=True)
    parser.add_argument('logs', nargs='*', type=Path)
    args = parser.parse_args()
    try:
        summary = aggregate(args.logs, args.expected_arms)
    except (OSError, ValueError) as exc:
        print(f'GATEB_TELEM ERROR: {exc}', file=sys.stderr)
        return 1
    print('GATEB_TELEM: ' + json.dumps(summary, sort_keys=True))
    return 0


if __name__ == '__main__':
    sys.exit(main())
