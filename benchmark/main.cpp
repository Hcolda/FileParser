#include "../Json.h"
#include <benchmark/benchmark.h>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <random>

long long generate_num() {
  static std::random_device rd;
  static std::mt19937_64 gen(rd());
  static std::uniform_int_distribution<long long> dis(0, INT64_MAX);
  return dis(gen);
}

void BM_MyJsonParse(benchmark::State &state) {
  const size_t array_size = state.range(0);
  qjson::JObject jobject;
  for (size_t i = 0; i < array_size; ++i) {
    jobject[std::to_string(i)] = std::to_string(generate_num());
  }
  std::string json = jobject.to_string();
  for (auto _ : state) {
    auto res = qjson::to_json(json);
    benchmark::DoNotOptimize(res);
  }

  state.SetComplexityN(array_size);
  state.SetBytesProcessed(json.size() * state.iterations());
}
BENCHMARK(BM_MyJsonParse)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Complexity();

void BM_MyJsonWrite(benchmark::State &state) {
  const size_t array_size = state.range(0);
  qjson::JObject jobject;
  for (size_t i = 0; i < array_size; ++i) {
    jobject[std::to_string(i)] = std::to_string(generate_num());
  }
  for (auto _ : state) {
    auto res = jobject.to_string();
    benchmark::DoNotOptimize(res);
  }

  state.SetComplexityN(array_size);
  state.SetBytesProcessed(jobject.to_string().size() * state.iterations());
}
BENCHMARK(BM_MyJsonWrite)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Complexity();

void BM_NlohmannJsonParse(benchmark::State &state) {
  const size_t array_size = state.range(0);
  qjson::JObject jobject;
  for (size_t i = 0; i < array_size; ++i) {
    jobject[std::to_string(i)] = std::to_string(generate_num());
  }
  std::string json = jobject.to_string();
  for (auto _ : state) {
    auto res = nlohmann::json::parse(json);
    benchmark::DoNotOptimize(res);
  }

  state.SetComplexityN(array_size);
  state.SetBytesProcessed(json.size() * state.iterations());
}
BENCHMARK(BM_NlohmannJsonParse)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Complexity();

void BM_NlohmannJsonWrite(benchmark::State &state) {
  const size_t array_size = state.range(0);
  nlohmann::json jobject;
  for (size_t i = 0; i < array_size; ++i) {
    jobject[std::to_string(i)] = std::to_string(generate_num());
  }
  for (auto _ : state) {
    auto res = jobject.dump();
    benchmark::DoNotOptimize(res);
  }

  state.SetComplexityN(array_size);
  state.SetBytesProcessed(jobject.dump().size() * state.iterations());
}
BENCHMARK(BM_NlohmannJsonWrite)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Complexity();

BENCHMARK_MAIN();
