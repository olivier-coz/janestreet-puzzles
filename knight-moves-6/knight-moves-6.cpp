#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <algorithm>
#include <climits>
#include <tuple>
#include <string>
#include <bitset>
#include <chrono>
#include <future>
#include <mutex>
#include <thread>

using namespace std;
using namespace chrono;

// Define the knight's possible moves
const vector<pair<int, int>> knight_moves = {
    {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
    {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
};

// Create the grid of letters
const char grid[6][6] = {
    {'a', 'a', 'a', 'b', 'b', 'c'},
    {'a', 'a', 'a', 'b', 'b', 'c'},
    {'a', 'a', 'b', 'b', 'c', 'c'},
    {'a', 'a', 'b', 'b', 'c', 'c'},
    {'a', 'b', 'b', 'c', 'c', 'c'},
    {'a', 'b', 'b', 'c', 'c', 'c'}
};

// Precompute knight moves for each position
vector<pair<int, int>> knight_moves_dict[6][6];

// Function to convert coordinates to chess notation
string to_chess_notation_string(const vector<pair<int, int>>& coords) {
    string notation;
    for (const auto& coord : coords) {
        char file = 'a' + coord.second;
        char rank = '1' + coord.first;
        notation += file;
        notation += rank;
        notation += ",";
    }
    if (!notation.empty()) notation.pop_back(); // Remove the trailing comma
    return notation;
}

// Function to calculate the score sequence
int calculate_score_sequence(const string& seq_letters, const unordered_map<char, int>& letter_to_value) {
    int score = letter_to_value.at(seq_letters[0]);
    for (size_t i = 1; i < seq_letters.size(); ++i) {
        char prev_letter = seq_letters[i - 1];
        char current_letter = seq_letters[i];
        int current_value = letter_to_value.at(current_letter);

        if (current_letter != prev_letter) {
            score *= current_value;
        } else {
            score += current_value;
        }

        // Early termination if score exceeds 2024
        if (score > 2024) {
            return -1;
        }
    }
    return score;
}

// Function to generate paths with lengths using parallel computing and avoiding duplicate sequences
void generate_paths_with_lengths_parallel_optimized(pair<int, int> start, pair<int, int> end, int max_steps,
    unordered_map<string, vector<pair<int, int>>>& sequences_dict) {

    struct Node {
        pair<int, int> current_pos;
        vector<pair<int, int>> path;
        bitset<36> visited; // Use bitset for visited positions
        string seq_letters;
    };

    // Prepare initial moves
    vector<Node> initial_nodes;
    bitset<36> initial_visited;
    int start_pos = start.first * 6 + start.second;
    initial_visited.set(start_pos);
    string initial_letter(1, grid[start.first][start.second]);
    vector<pair<int, int>> initial_path = { start };

    // Generate initial nodes based on knight moves from the starting position
    for (const auto& next_pos : knight_moves_dict[start.first][start.second]) {
        bitset<36> visited = initial_visited;
        int pos_index = next_pos.first * 6 + next_pos.second;
        visited.set(pos_index);
        string seq_letters = initial_letter + grid[next_pos.first][next_pos.second];
        vector<pair<int, int>> path = initial_path;
        path.push_back(next_pos);
        initial_nodes.push_back({ next_pos, move(path), visited, move(seq_letters) });
    }

    // Use a thread pool to process initial nodes in parallel
    const unsigned int num_threads = thread::hardware_concurrency();
    vector<future<unordered_map<string, vector<pair<int, int>>>>> futures;

    size_t batch_size = (initial_nodes.size() + num_threads - 1) / num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t start_idx = i * batch_size;
        size_t end_idx = min(start_idx + batch_size, initial_nodes.size());

        futures.push_back(async(launch::async, [&, start_idx, end_idx]() {
            unordered_map<string, vector<pair<int, int>>> local_sequences_dict;
            unordered_set<string> seen_sequences;
            stack<Node> s;

            // Push initial nodes for this thread
            for (size_t idx = start_idx; idx < end_idx; ++idx) {
                Node& node = initial_nodes[idx];
                seen_sequences.insert(node.seq_letters);
                s.push(move(node));
            }

            while (!s.empty()) {
                Node node = s.top();
                s.pop();

                if (node.path.size() > max_steps + 1) continue;

                if (node.current_pos == end && node.path.size() >= 2) {
                    local_sequences_dict[node.seq_letters] = node.path;
                    continue;
                }

                for (const auto& next_pos : knight_moves_dict[node.current_pos.first][node.current_pos.second]) {
                    int pos_index = next_pos.first * 6 + next_pos.second;
                    if (!node.visited.test(pos_index)) {
                        bitset<36> new_visited = node.visited;
                        new_visited.set(pos_index);
                        string new_seq_letters = node.seq_letters + grid[next_pos.first][next_pos.second];

                        // Check if the sequence has been seen
                        if (seen_sequences.find(new_seq_letters) != seen_sequences.end()) {
                            continue; // Skip this path as it leads to a duplicate sequence
                        }

                        seen_sequences.insert(new_seq_letters);

                        vector<pair<int, int>> new_path = node.path;
                        new_path.push_back(next_pos);
                        s.push({ next_pos, move(new_path), new_visited, move(new_seq_letters) });
                    }
                }
            }

            return local_sequences_dict;
        }));
    }

    // Collect results from all threads
    for (auto& fut : futures) {
        unordered_map<string, vector<pair<int, int>>> local_dict = fut.get();
        for (auto& pair : local_dict) {
            sequences_dict[pair.first] = move(pair.second);
        }
    }
}

int main() {
    // Measure total time
    auto total_start = high_resolution_clock::now();

    // Precompute knight moves for each position
    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 6; ++y) {
            for (const auto& move : knight_moves) {
                int new_x = x + move.first;
                int new_y = y + move.second;
                if (new_x >= 0 && new_x < 6 && new_y >= 0 && new_y < 6) {
                    knight_moves_dict[x][y].emplace_back(new_x, new_y);
                }
            }
        }
    }

    // Generate paths from a1 to f6 and measure time
    auto start_time_a1_f6 = high_resolution_clock::now();
    pair<int, int> start_a1 = { 0, 0 }, end_f6 = { 5, 5 };
    unordered_map<string, vector<pair<int, int>>> sequences_dict_a1_to_f6;
    cout << "Generating paths from a1 to f6..." << endl;
    generate_paths_with_lengths_parallel_optimized(start_a1, end_f6, 34, sequences_dict_a1_to_f6);
    cout << "Number of unique sequences from a1 to f6: " << sequences_dict_a1_to_f6.size() << endl;
    auto end_time_a1_f6 = high_resolution_clock::now();
    auto duration_a1_f6 = duration_cast<milliseconds>(end_time_a1_f6 - start_time_a1_f6).count();
    cout << "Time for Generating paths from a1 to f6: " << duration_a1_f6 << " ms" << endl;

    // Generate paths from a6 to f1 and measure time
    auto start_time_a6_f1 = high_resolution_clock::now();
    pair<int, int> start_a6 = { 5, 0 }, end_f1 = { 0, 5 };
    unordered_map<string, vector<pair<int, int>>> sequences_dict_a6_to_f1;
    cout << "Generating paths from a6 to f1..." << endl;
    generate_paths_with_lengths_parallel_optimized(start_a6, end_f1, 34, sequences_dict_a6_to_f1);
    cout << "Number of unique sequences from a6 to f1: " << sequences_dict_a6_to_f1.size() << endl;
    auto end_time_a6_f1 = high_resolution_clock::now();
    auto duration_a6_f1 = duration_cast<milliseconds>(end_time_a6_f1 - start_time_a6_f1).count();
    cout << "Time for Generating paths from a6 to f1: " << duration_a6_f1 << " ms" << endl;

    // Measure time for finding the best combination
    auto start_time_best_combination = high_resolution_clock::now();

    // Possible values for A, B, and C
    vector<int> possible_values = { 1, 2, 3 };

    int min_A_B_C_sum = INT_MAX;
    vector<int> best_combination;
    vector<tuple<int, int, int, vector<pair<int, int>>, vector<pair<int, int>>>> matching_paths;

    // Generate all possible combinations of A, B, and C (including repetitions)
    vector<vector<int>> combinations;
    for (int A : possible_values) {
        for (int B : possible_values) {
            for (int C : possible_values) {
                combinations.push_back({ A, B, C });
            }
        }
    }

    // Prepare sequences for processing
    vector<string> sequences_a1_f6_keys;
    sequences_a1_f6_keys.reserve(sequences_dict_a1_to_f6.size());
    for (const auto& seq : sequences_dict_a1_to_f6) {
        sequences_a1_f6_keys.push_back(seq.first);
    }

    vector<string> sequences_a6_f1_keys;
    sequences_a6_f1_keys.reserve(sequences_dict_a6_to_f1.size());
    for (const auto& seq : sequences_dict_a6_to_f1) {
        sequences_a6_f1_keys.push_back(seq.first);
    }

    // Process combinations
    for (const auto& comb : combinations) {
        int A = comb[0], B = comb[1], C = comb[2];
        int current_sum = A + B + C;

        if (current_sum > min_A_B_C_sum) continue;

        unordered_map<char, int> letter_to_value = { {'a', A}, {'b', B}, {'c', C} };

        vector<string> valid_sequences_a1_f6;
        valid_sequences_a1_f6.reserve(sequences_a1_f6_keys.size());
        for (const auto& seq_letters : sequences_a1_f6_keys) {
            int score = calculate_score_sequence(seq_letters, letter_to_value);
            if (score == 2024) {
                valid_sequences_a1_f6.push_back(seq_letters);
            }
        }

        if (!valid_sequences_a1_f6.empty()) {
            vector<string> valid_sequences_a6_f1;
            valid_sequences_a6_f1.reserve(sequences_a6_f1_keys.size());
            for (const auto& seq_letters : sequences_a6_f1_keys) {
                int score = calculate_score_sequence(seq_letters, letter_to_value);
                if (score == 2024) {
                    valid_sequences_a6_f1.push_back(seq_letters);
                }
            }

            if (!valid_sequences_a6_f1.empty()) {
                for (const auto& seq_a1_f6 : valid_sequences_a1_f6) {
                    for (const auto& seq_a6_f1 : valid_sequences_a6_f1) {
                        matching_paths.emplace_back(A, B, C, sequences_dict_a1_to_f6[seq_a1_f6], sequences_dict_a6_to_f1[seq_a6_f1]);
                        if (current_sum < min_A_B_C_sum) {
                            min_A_B_C_sum = current_sum;
                            best_combination = { A, B, C };
                        }
                    }
                }
            }
        }
    }

    auto end_time_best_combination = high_resolution_clock::now();
    auto duration_best_combination = duration_cast<milliseconds>(end_time_best_combination - start_time_best_combination).count();
    cout << "Time for finding Best combination: " << duration_best_combination << " ms" << endl;

    auto total_end = high_resolution_clock::now();
    auto total_duration = duration_cast<milliseconds>(total_end - total_start).count();
    cout << "Total Time: " << total_duration << " ms" << endl;

    if (!matching_paths.empty()) {
        cout << "\nBest combination: A=" << best_combination[0] << ", B=" << best_combination[1] << ", C=" << best_combination[2] << endl;
        cout << "Minimum A+B+C sum: " << min_A_B_C_sum << endl;
        cout << "Number of matching paths: " << matching_paths.size() << endl;
        auto [A, B, C, path_a1_f6, path_a6_f1] = matching_paths[0];
        cout << "First matching path from a1 to f6: " << to_chess_notation_string(path_a1_f6) << endl;
        cout << "First matching path from a6 to f1: " << to_chess_notation_string(path_a6_f1) << endl;
    }
    else {
        cout << "No matching paths found." << endl;
    }

    return 0;
}

//(base) [user@lenovo14 knight-moves-6]$ g++ -std=c++23 -O3 -march=native knight-moves-6.cpp && ./a.out
// Generating paths from a1 to f6...
// Number of unique sequences from a1 to f6: 474346
// Time for Generating paths from a1 to f6: 4905 ms
// Generating paths from a6 to f1...
// Number of unique sequences from a6 to f1: 169214
// Time for Generating paths from a6 to f1: 6119 ms
// Time for finding Best combination: 277 ms
// Total Time: 11302 ms

// Best combination: A=1, B=2, C=1
// Minimum A+B+C sum: 4
// Number of matching paths: 492
// First matching path from a1 to f6: a1,c2,d4,c6,b4,a6,c5,e6,f4,e2,c1,b3,d2,b1,a3,b5,d6,c4,e5,d3,b2,a4,b6,d5,e3,d1,f2,e4,f6
// First matching path from a6 to f1: a6,c5,b3,a5,c6,e5,f3,e1,c2,e3,d1,f2,e4,f6,d5,b4,d3,c1,e2,f4,e6,d4,b5,a3,c4,b6,a4,c3,b1,d2,f1
