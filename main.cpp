#include <cstdio>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) { return 1; }

    std::vector<int> graph(n * n);
    std::vector<int> edge_count(n);

    for (int i = 0; i < m; ++i) {
        int u, v;
        if (scanf("%d %d", &u, &v) != 2) { return 1; }
        if (u > n || v > n) { return 1; }

        --u;
        --v;

        if (graph[u * n + v] == 0) {
            graph[u * n + v] = 1;
            graph[v * n + u] = 1;
            ++edge_count[u];
            ++edge_count[v];
        }
    }

    std::vector<int> visited(n);
    std::vector<int> buffer{0};

    int edge_num = 0, node_num = 0, sum = 0;

    while(!buffer.empty()) {
        int node = buffer.back();
        buffer.pop_back();

        if (visited[node] == 0) {
            visited[node] = 1;
            ++node_num;
            edge_num += edge_count[node];

            for (int i = 0; i < n; ++i) {
                if (graph[node * n + i] > 0 && visited[i] == 0) {
                    buffer.emplace_back(node);
                    buffer.emplace_back(i);

                    break;
                }
            }
        } else if (!buffer.empty()) {
            int prev = buffer.back();
            buffer.pop_back();

            for (int i = node + 1; i < n; ++i) {
                if (graph[prev * n + i] > 0 && visited[i] == 0) {
                    buffer.emplace_back(prev);
                    buffer.emplace_back(i);

                    break;
                }
            }
        } else {
            int score = edge_num / 2 - node_num;
            if (score > 0) { sum += score; }

            edge_num = 0;
            node_num = 0;

            for (int i = node + 1; i < n; ++i) {
                if (visited[i] == 0) {
                    buffer.emplace_back(i);
                    break;
                }
            }
        }
    }

    printf("%d\n", sum);
}
