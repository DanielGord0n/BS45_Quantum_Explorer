/*
 * Tabu Search for Base Sequences BS(n)
 * CP493 - Directed Research - Daniel Gordon
 *
 * Finds four sequences A, B, C, D of length n with entries +/-1
 * such that NPAF (Non-Periodic Autocorrelation Function) = 0
 * for all shifts s = 1, ..., n-1.
 *
 * Key optimizations:
 *   - O(n) delta evaluation per flip (instead of O(n^2) full recalc)
 *   - Maintains running correlation array for instant energy updates
 *   - Multiple random restarts to escape deep local minima
 *   - Configurable tabu tenure and iteration limits
 *
 * Compile: g++ -O3 -std=c++17 -o tabu_search_bs tabu_search_bs.cpp
 * Usage:   ./tabu_search_bs <n> [max_restarts] [iters_per_restart]
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

static int N;                       // sequence length
static int seqs[4][256];            // 4 sequences, max length 256
static int corr[256];               // corr[s] = sum of autocorrelations at shift s
static int tabu[4][256];            // tabu expiry iteration for each position
static int best_seqs[4][256];       // best solution found
static long long best_energy_global;

static mt19937 rng;

// Calculate full energy from scratch (sum of corr[s]^2)
long long full_energy() {
    long long e = 0;
    for (int s = 1; s < N; s++) {
        int c = 0;
        for (int j = 0; j < 4; j++)
            for (int i = 0; i < N - s; i++)
                c += seqs[j][i] * seqs[j][i + s];
        corr[s] = c;
        e += (long long)c * c;
    }
    return e;
}

// Delta energy if we flip seqs[seq_idx][pos]
// Returns the change in energy (new_energy - old_energy)
// Also fills delta_corr[] for the caller to apply
static int delta_corr_buf[256];

long long delta_energy(int seq_idx, int pos) {
    long long delta_e = 0;
    for (int s = 1; s < N; s++) {
        // How does corr[s] change when we flip seqs[seq_idx][pos]?
        // Terms involving pos:
        //   if pos + s < N: seqs[seq_idx][pos] * seqs[seq_idx][pos+s]
        //   if pos - s >= 0: seqs[seq_idx][pos-s] * seqs[seq_idx][pos]
        // After flip, each of these terms changes sign, so delta = -2 * term
        int dc = 0;
        if (pos + s < N)
            dc += seqs[seq_idx][pos] * seqs[seq_idx][pos + s];
        if (pos - s >= 0)
            dc += seqs[seq_idx][pos - s] * seqs[seq_idx][pos];
        dc *= -2;
        delta_corr_buf[s] = dc;

        long long old_c = corr[s];
        long long new_c = old_c + dc;
        delta_e += new_c * new_c - old_c * old_c;
    }
    return delta_e;
}

// Apply a flip: update seqs, corr, and energy
void apply_flip(int seq_idx, int pos) {
    // Update correlations first (delta_corr_buf must be filled by delta_energy)
    for (int s = 1; s < N; s++)
        corr[s] += delta_corr_buf[s];
    // Flip the element
    seqs[seq_idx][pos] *= -1;
}

void randomize_seqs() {
    uniform_int_distribution<int> dist(0, 1);
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < N; i++)
            seqs[j][i] = dist(rng) ? 1 : -1;
}

long long tabu_search(int max_iters, int tabu_tenure) {
    long long current_energy = full_energy();
    long long best_energy = current_energy;

    // Save initial as best
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < N; i++)
            best_seqs[j][i] = seqs[j][i];

    // Clear tabu list
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < N; i++)
            tabu[j][i] = 0;

    for (int iter = 0; iter < max_iters; iter++) {
        if (current_energy == 0) break;

        long long best_delta = LLONG_MAX;
        int best_j = -1, best_i = -1;

        // Evaluate all 4*N neighbors
        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < N; i++) {
                long long de = delta_energy(j, i);
                bool is_tabu = (tabu[j][i] > iter);

                // Accept if: not tabu, or aspiration (beats global best)
                if (de < best_delta && (!is_tabu || (current_energy + de) < best_energy)) {
                    best_delta = de;
                    best_j = j;
                    best_i = i;
                }
            }
        }

        if (best_j < 0) break; // no move found (shouldn't happen)

        // Recompute delta for the chosen move (delta_corr_buf needs to be set)
        delta_energy(best_j, best_i);
        apply_flip(best_j, best_i);
        current_energy += best_delta;

        // Update tabu list
        tabu[best_j][best_i] = iter + tabu_tenure;

        if (current_energy < best_energy) {
            best_energy = current_energy;
            for (int j = 0; j < 4; j++)
                for (int i = 0; i < N; i++)
                    best_seqs[j][i] = seqs[j][i];

            if (iter % 100 == 0 || current_energy < 100)
                cout << "  Iter " << iter << ": Energy = " << best_energy << endl;
        }

        if (current_energy == 0) break;
    }

    return best_energy;
}

bool verify_solution(int sol[4][256], int n) {
    for (int s = 1; s < n; s++) {
        int c = 0;
        for (int j = 0; j < 4; j++)
            for (int i = 0; i < n - s; i++)
                c += sol[j][i] * sol[j][i + s];
        if (c != 0) return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <n> [max_restarts] [iters_per_restart]" << endl;
        return 1;
    }

    N = atoi(argv[1]);
    if (N < 2 || N > 256) {
        cerr << "n must be between 2 and 256" << endl;
        return 1;
    }

    int max_restarts = 100;
    int iters_per_restart = 50000;
    if (argc > 2) max_restarts = atoi(argv[2]);
    if (argc > 3) iters_per_restart = atoi(argv[3]);

    int tabu_tenure = max(7, N / 4);

    rng.seed(chrono::steady_clock::now().time_since_epoch().count());

    cout << "=== Tabu Search for BS(" << N << ") ===" << endl;
    cout << "Search space: 2^" << 4 * N << " possibilities" << endl;
    cout << "Restarts: " << max_restarts << ", Iters/restart: " << iters_per_restart << endl;
    cout << "Tabu tenure: " << tabu_tenure << endl;
    cout << endl;

    best_energy_global = LLONG_MAX;
    auto t_start = chrono::steady_clock::now();

    for (int restart = 0; restart < max_restarts; restart++) {
        randomize_seqs();
        long long e = tabu_search(iters_per_restart, tabu_tenure);

        if (e < best_energy_global) {
            best_energy_global = e;
            cout << "Restart " << restart << ": New global best energy = " << best_energy_global << endl;
        }

        if (best_energy_global == 0) {
            cout << "\nSUCCESS! Found BS(" << N << ")!" << endl;
            break;
        }

        // Adaptive: increase tenure if stuck
        if (restart > 0 && restart % 10 == 0 && best_energy_global > 0) {
            tabu_tenure = min(tabu_tenure + 2, N);
        }
    }

    auto t_end = chrono::steady_clock::now();
    double elapsed = chrono::duration<double>(t_end - t_start).count();

    cout << "\n=== Results ===" << endl;
    cout << "Time: " << elapsed << " seconds" << endl;
    cout << "Best energy: " << best_energy_global << endl;

    if (best_energy_global == 0 && verify_solution(best_seqs, N)) {
        cout << "VERIFIED: Valid BS(" << N << ")!" << endl;
        const char *names[] = {"A", "B", "C", "D"};
        for (int j = 0; j < 4; j++) {
            cout << names[j] << " = [";
            for (int i = 0; i < N; i++) {
                if (i > 0) cout << ",";
                cout << best_seqs[j][i];
            }
            cout << "]" << endl;
        }
    } else {
        cout << "Did not find a perfect solution." << endl;
        cout << "Best energy " << best_energy_global << " (needs to be 0)." << endl;
    }

    return (best_energy_global == 0) ? 0 : 1;
}
