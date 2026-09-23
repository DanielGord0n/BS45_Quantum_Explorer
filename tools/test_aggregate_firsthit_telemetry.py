#!/usr/bin/env python3
"""Fixture-only tests for pooled first-hit telemetry; no solver execution."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).with_name('aggregate_firsthit_telemetry.py')


def record(**changes):
    r = dict(version=2, n=44, stride=1, worker_ns=100, cell_ns=90,
             score_ns_est=10, sort_ns=10, complete_ns=50, score_calls=8,
             score_samples=8, completions=2, hist_candidates=2, hist_nodes=10,
             total_nodes=10, depth_nodes=[0]*23, rank_nodes=[10]+[0]*9,
             rank_count=[2]+[0]*9, rank_ns=[50]+[0]*9, rank_aborts=[1]+[0]*9,
             unranked_nodes=0, unranked_count=0, unranked_ns=0,
             cells_live_done=1, cells_partial=0, resume_replay_ns=0,
             resume_replay_score_ns_est=0, resume_replay_sort_ns=0, resume_cells=0)
    r['depth_nodes'][11] = 4
    r['depth_nodes'][12] = 6
    r.update(changes)
    return r


class AggregateTest(unittest.TestCase):
    def run_logs(self, logs, expected=None, duplicate=False):
        with tempfile.TemporaryDirectory() as tmp:
            paths = []
            for i, records in enumerate(logs):
                path = Path(tmp)/f'arm_{i}.log'
                path.write_text('\n'.join('FH_TELEM '+json.dumps(r) if isinstance(r, dict)
                                          else r for r in records)+'\n')
                paths.append(str(path))
            if duplicate:
                paths.append(paths[0])
            p = subprocess.run([sys.executable, str(SCRIPT), '--expected-arms',
                                str(expected if expected is not None else len(logs)),
                                *paths], capture_output=True, text=True)
            return p

    def summary(self, *args, **kwargs):
        p = self.run_logs(*args, **kwargs)
        self.assertEqual(p.returncode, 0, p.stderr)
        self.assertTrue(p.stdout.startswith('GATEB_TELEM: '), p.stdout)
        return json.loads(p.stdout.removeprefix('GATEB_TELEM: '))

    def test_last_record_only_and_weighted_shares(self):
        r = record(worker_ns=900, cell_ns=800, complete_ns=700,
                   rank_ns=[700]+[0]*9)
        s = self.summary([['FH_TELEM {invalid', record(), r], [record()]])
        self.assertEqual(s['worker_ns'], 1000)
        self.assertEqual(s['completion_share'], .75)
        self.assertEqual(s['g'], .14)
        self.assertEqual(s['other_ns'], 110)
        self.assertEqual(s['stream_gross_ns'], 120)
        self.assertTrue(s['complete_coverage'])

    def test_missing_and_partial_are_explicit(self):
        s = self.summary([[record(cells_partial=1)], ['unfinished']], expected=3)
        self.assertFalse(s['complete_coverage'])
        self.assertEqual(s['arms_reported'], 1)
        self.assertEqual(s['arms_missing'], 2)
        self.assertEqual(s['cells_partial'], 1)
        self.assertEqual(len(s['missing_record_paths']), 1)
        self.assertEqual(s['missing_input_arms'], 1)

    def test_no_records_has_null_shares(self):
        s = self.summary([['unfinished']])
        self.assertIsNone(s['g'])
        self.assertIsNone(s['late_share'])
        self.assertFalse(s['complete_coverage'])

    def test_depth_boundary(self):
        s = self.summary([[record()]])
        self.assertEqual(s['late_nodes'], 10)  # n44: d11 leaves 10 quads, d12 leaves 9
        r = record(depth_nodes=[0]*10+[4,0,6]+[0]*10)
        self.assertEqual(self.summary([[r]])['late_share'], .6)

    def test_sampled_estimate_can_exceed_residual(self):
        r = record(stride=64, score_samples=1, score_ns_est=60,
                   hist_candidates=1, hist_nodes=6, depth_nodes=[0]*12+[6]+[0]*10)
        s = self.summary([[r]])
        self.assertFalse(s['decomposition_valid'])
        self.assertEqual(s['stream_other_ns_est'], -30)
        self.assertEqual(s['late_share'], 1)
        self.assertEqual(s['histogram_mode'], 'sampled')
        self.assertFalse(s['stats_sufficient'])
        self.assertEqual(s['stats_criterion'], 'sampled insufficient')

    def test_thresholds_use_raw_histogram_nodes(self):
        r = record(cells_live_done=500, completions=25000, hist_candidates=25000,
                   rank_count=[25000]+[0]*9, total_nodes=10**9, hist_nodes=10**9,
                   rank_nodes=[10**9]+[0]*9, depth_nodes=[10**9]+[0]*22)
        self.assertTrue(self.summary([[r]])['stats_sufficient'])
        r.update(stride=64, hist_candidates=1, hist_nodes=10, depth_nodes=[10]+[0]*22)
        self.assertFalse(self.summary([[r]])['stats_sufficient'])

    def test_reject_invalid_records(self):
        bad = [dict(rank_nodes=[10]), dict(hist_nodes=11), dict(total_nodes=11),
               dict(completions=3), dict(rank_ns=[49]+[0]*9),
               dict(rank_aborts=[3]+[0]*9), dict(worker_ns=89),
               dict(cell_ns=59), dict(score_samples=9), dict(score_calls=True),
               dict(hist_candidates=1), dict(stride=2), dict(version=1),
               dict(depth_nodes=[-1]+[0]*22)]
        for changes in bad:
            with self.subTest(changes=changes):
                s = self.summary([[record(**changes)]])
                self.assertEqual(s['arms_missing'], 1)
                self.assertEqual(len(s['rejected_records']), 1)
                self.assertFalse(s['coverage_acceptable'])
        for logs in [[[record()], [record(n=43)]],
                     [[record()], [record(stride=64)]]]:
            self.assertNotEqual(self.run_logs(logs).returncode, 0)
        self.assertNotEqual(self.run_logs([[record()]], duplicate=True).returncode, 0)
        self.assertNotEqual(self.run_logs([[record()], [record()]], expected=1).returncode, 0)

    def test_replay_subtraction_pooled_sampled_shares(self):
        a = record(stride=64, resume_replay_ns=20, resume_replay_score_ns_est=5,
                   resume_replay_sort_ns=4, resume_cells=1)
        b = record(stride=64, worker_ns=900, cell_ns=800, complete_ns=700,
                   rank_ns=[700]+[0]*9, resume_replay_ns=40,
                   resume_replay_score_ns_est=3, resume_replay_sort_ns=6,
                   resume_cells=2)
        s = self.summary([[a], [b]])
        self.assertEqual(s['worker_ns_without_replay'], 940)
        self.assertEqual(s['cell_ns_without_replay'], 830)
        self.assertEqual(s['score_ns_est_without_replay'], 12)
        self.assertEqual(s['sort_ns_without_replay'], 10)
        self.assertEqual(s['stream_other_ns_est_without_replay'], 58)
        self.assertEqual(s['g_without_replay'], 80/940)
        self.assertEqual(s['scoring_share_without_replay'], 12/940)
        self.assertEqual(s['sort_share_without_replay'], 10/940)
        self.assertEqual(s['completion_share_without_replay'], 750/940)
        self.assertEqual(s['other_share_without_replay'], 110/940)
        self.assertEqual(s['other_share'], .11)
        self.assertEqual(s['replay_share'], .06)
        self.assertEqual(s['resume_cells'], 3)
        self.assertTrue(s['decomposition_without_replay_valid'])

    def test_zero_fresh_interval(self):
        r = record(worker_ns=100, cell_ns=100, complete_ns=0, rank_ns=[0]*10,
                   resume_replay_ns=100, resume_replay_sort_ns=10,
                   resume_replay_score_ns_est=10)
        s = self.summary([[r]])
        for key in ('g', 'scoring_share', 'sort_share', 'completion_share', 'other_share'):
            self.assertIsNone(s[key+'_without_replay'])
        self.assertEqual(s['replay_share'], 1)

    def test_coverage_policy_and_corrupt_arm(self):
        logs = [[record()], ['unfinished'], ['FH_TELEM {bad']]
        s = self.summary(logs, expected=4)
        self.assertTrue(s['coverage_acceptable'])
        self.assertFalse(s['complete_coverage'])
        self.assertEqual(s['worker_ns'], 100)
        self.assertEqual(s['arms_missing'], 3)
        self.assertEqual(len(s['rejected_records']), 1)
        self.assertIn(s['rejected_records'][0]['path'], s['missing_record_paths'])
        self.assertTrue(s['rejected_records'][0]['reason'])
        self.assertTrue(s['missing_arms_bias_warning'])
        self.assertFalse(self.summary(logs, expected=5)['coverage_acceptable'])
        s = self.summary([['FH_TELEM {bad'], [record(version=1)]])
        self.assertFalse(s['coverage_acceptable'])
        self.assertFalse(s['complete_coverage'])
        self.assertFalse(s['pilot_read_usable'])
        self.assertEqual(s['arms_reported'], 0)

    def test_replay_counter_validation(self):
        invalid = [dict(resume_replay_ns=-1), dict(resume_cells=True),
                   dict(resume_replay_ns=41, resume_replay_sort_ns=10),
                   dict(resume_replay_ns=15, resume_replay_sort_ns=11),
                   dict(resume_replay_ns=2, resume_replay_sort_ns=3),
                   dict(resume_replay_score_ns_est=11), dict(score_ns_est=31),
                   dict(resume_replay_ns=2, resume_replay_score_ns_est=3),
                   dict(resume_replay_ns=25, resume_replay_score_ns_est=0)]
        for changes in invalid:
            with self.subTest(changes=changes):
                s = self.summary([[record(**changes)]])
                self.assertEqual(len(s['rejected_records']), 1)
        s = self.summary([[record(stride=64, resume_replay_ns=25)]])
        self.assertTrue(s['decomposition_valid'])
        self.assertFalse(s['decomposition_without_replay_valid'])
        self.assertEqual(s['stream_other_ns_est_without_replay'], -5)

    def test_pilot_requires_coverage_volume_and_decomposition(self):
        r = record(cells_live_done=500, completions=25000, hist_candidates=25000,
                   rank_count=[25000]+[0]*9, total_nodes=10**9, hist_nodes=10**9,
                   rank_nodes=[10**9]+[0]*9, depth_nodes=[10**9]+[0]*22)
        self.assertTrue(self.summary([[r]], expected=4)['pilot_read_usable'])
        s = self.summary([[r]], expected=5)
        self.assertTrue(s['stats_sufficient'])
        self.assertFalse(s['pilot_read_usable'])
        r.update(stride=64, resume_replay_ns=25)
        self.assertFalse(self.summary([[r]])['pilot_read_usable'])

    def test_extra_fields_ignored(self):
        self.assertEqual(self.summary([[record(future_field='ignored')]])['completions'], 2)


if __name__ == '__main__':
    unittest.main()
