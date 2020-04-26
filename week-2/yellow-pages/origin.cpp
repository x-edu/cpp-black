#include "yellow_pages.h"

#include <algorithm>
#include <string>

using namespace std;

namespace YellowPages {

static void MergeSingleField(
    const string& name,
    const Signals& signals,
    const Providers& providers,
    Company& result
) {
  auto field = result.GetDescriptor()->FindFieldByName(name);
  string serialized_value;
  int priority = 0;
  for (const auto& signal : signals) {
    const auto& provider = providers.at(signal.provider_id());
    const auto& company = signal.company();
    auto ref = company.GetReflection();
    if (!ref->HasField(company, field)) {
      continue;
    }
    if (provider.priority() > priority) {
      priority = provider.priority();
      serialized_value = ref->GetMessage(company, field).SerializeAsString();
    }
  }
  if (!serialized_value.empty()) {
    auto item = result.GetReflection()->MutableMessage(&result, field);
    item->ParseFromString(serialized_value);
  }
}

static void MergeRepeatedField(
    const string& name,
    const Signals& signals,
    const Providers& providers,
    Company& result
) {
  auto field = result.GetDescriptor()->FindFieldByName(name);
  set<string> values;
  int priority = 0;
  for (const auto& signal : signals) {
    const auto& provider = providers.at(signal.provider_id());
    const auto& company = signal.company();
    auto ref = company.GetReflection();
    auto sz = ref->FieldSize(company, field);
    if (provider.priority() < priority || sz == 0) {
      continue;
    }
    if (provider.priority() > priority) {
      priority = provider.priority();
      values.clear();
    }
    for (int i = 0; i != sz; ++i) {
      values.insert(ref->GetRepeatedMessage(company, field, i).SerializeAsString());
    }
  }
  for (const auto& serialized_value : values) {
    auto item = result.GetReflection()->AddMessage(&result, field);
    item->ParseFromString(serialized_value);
  }
}

Company Merge(const Signals& signals, const Providers& providers) {
  Company result;
  MergeSingleField("address", signals, providers, result);
  MergeRepeatedField("names", signals, providers, result);
  MergeRepeatedField("phones", signals, providers, result);
  MergeRepeatedField("urls", signals, providers, result);
  MergeSingleField("working_time", signals, providers, result);
  return result;
}

}
