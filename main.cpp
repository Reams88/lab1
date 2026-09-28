// компілятор: GCC 16
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <map>
#include <numeric>
#include <compare>
#include <optional>
#include <print>
#include <stdexcept>

using namespace std;

struct Dom {
  int a, b;
  auto operator<=>(const Dom&) const = default;
};

class Dlr {
  vector<Dom> pool;
  mt19937 rng;

public:
  Dlr(int n, unsigned seed) : rng(seed) {
    for (int i = 0; i <= n; ++i) {
      for (int j = i; j <= n; ++j) {
        pool.push_back({i, j});
      }
    }
  }

  optional<Dom> operator()() {
    if (pool.empty()) return nullopt;
    
    uniform_int_distribution<size_t> dist(0, pool.size() - 1);
    size_t idx = dist(rng);
    
    Dom d = pool[idx];
    pool[idx] = pool.back();
    pool.pop_back();
    
    return d;
  }
};

int runDeal(int n, unsigned seed) {
  Dlr dlr(n, seed);
  
  auto first = dlr();
  if (!first) return 0;
  
  int e1 = first->a;
  int e2 = first->b;
  int sz = 1;

  while (auto next = dlr()) {
    Dom d = *next;
    
    bool canE1a = (d.a == e1), canE1b = (d.b == e1);
    bool canE2a = (d.a == e2), canE2b = (d.b == e2);

    if (!canE1a && !canE1b && !canE2a && !canE2b) {
      break;
    }

    vector<int> opts;
    if (canE1a || canE2a) opts.push_back(d.a);
    if (canE1b || canE2b) opts.push_back(d.b);

    int attachNum = *min_element(opts.begin(), opts.end());

    if (attachNum == d.a) {
      if (canE1a) e1 = d.b; 
      else e2 = d.b;        
    } else {
      if (canE1b) e1 = d.a;
      else e2 = d.a;
    }

    sz++;
  }
  
  return sz;
}

struct Stats {
  int mode;
  double mean;
  double median;
  map<int, int> freqs;
};

Stats runSim(int n, int nDeals) {
  vector<int> szs;
  szs.reserve(nDeals);
  
  random_device rd;
  unsigned baseSeed = rd();

  for (int i = 0; i < nDeals; ++i) {
    szs.push_back(runDeal(n, baseSeed + i));
  }

  Stats st;
  for (int s : szs) st.freqs[s]++;

  auto modeIt = max_element(st.freqs.begin(), st.freqs.end(), [](const auto& p1, const auto& p2) {
    return p1.second < p2.second;
  });
  st.mode = modeIt->first;

  st.mean = accumulate(szs.begin(), szs.end(), 0.0) / nDeals;

  sort(szs.begin(), szs.end(), greater<int>{});
  
  if (nDeals % 2 == 0) {
    st.median = (szs[nDeals / 2 - 1] + szs[nDeals / 2]) / 2.0;
  } else {
    st.median = szs[nDeals / 2];
  }

  return st;
}

int main() {
  try {
    int mode;
    print("1. Single run\n2. Analysis\nMode: ");
    if (!(cin >> mode)) throw invalid_argument("must be a number");
    if (mode != 1 && mode != 2) throw invalid_argument("must be 1 or 2");

    if (mode == 1) {
      int n, nDeals;
      print("n (>= 0): ");
      if (!(cin >> n) || n < 0) throw invalid_argument("must be >= 0");
      
      print("Deals (> 0): ");
      if (!(cin >> nDeals) || nDeals <= 0) throw invalid_argument("must be > 0");
      
      Stats st = runSim(n, nDeals);
      
      println("\nDistribution:");
      for (auto const& [s, count] : st.freqs) {
        println("Size {:2d}: {:5.2f}%", s, (count * 100.0) / nDeals);
      }
      println("\nMode   : {}\nMean   : {:.2f}\nMedian : {:.2f}", st.mode, st.mean, st.median);
    } 
    else if (mode == 2) {
      int maxN, nDeals;
      print("Maximum n (>= 1): ");
      if (!(cin >> maxN) || maxN < 1) throw invalid_argument("must be >= 1.");
      
      print("Deals per n (> 0): ");
      if (!(cin >> nDeals) || nDeals <= 0) throw invalid_argument("must be > 0.");

      println("\n  n | Total Tiles | Mean/Total | Median/Total"); // 45
      println("---------------------------------------------"); // 45
      for (int n = 1; n <= maxN; ++n) {
        Stats st = runSim(n, nDeals);
        int totalTiles = (n + 1) * (n + 2) / 2;
        double meanRatio = st.mean / totalTiles;
        double medianRatio = st.median / totalTiles;
        
        println("{:3d} | {:11d} | {:10.4f} | {:12.4f}", n, totalTiles, meanRatio, medianRatio); //3+11+10+12=36+3*3=45
      }
    }
  } catch (const exception& e) {
    println("Error: {}", e.what());
    return 1;
  }

  return 0;
}
