#include "SeaDix.h"

Napi::FunctionReference SeaDix::constructor;

Napi::Object SeaDix::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);

	Napi::Function func = DefineClass(
		env, "SeaDix",
		{InstanceMethod("insert", &SeaDix::Insert),
		 InstanceMethod("search", &SeaDix::Search),
		 InstanceMethod("remove", &SeaDix::Remove),
		 InstanceMethod("startsWith", &SeaDix::StartsWith),
		 InstanceMethod("wordsWithPrefix", &SeaDix::WordsWithPrefix),
		 InstanceMethod("insertBatch", &SeaDix::InsertBatch),
		 InstanceMethod("searchBatch", &SeaDix::SearchBatch),
		 InstanceMethod("removeBatch", &SeaDix::RemoveBatch),
		 InstanceMethod("insertFromFile", &SeaDix::InsertFromFile),
		 InstanceMethod("removeFromFile", &SeaDix::RemoveFromFile),
		 InstanceMethod("searchFromFile", &SeaDix::SearchFromFile),
		 InstanceMethod("setNormalization", &SeaDix::SetNormalization),
		 InstanceMethod("setAllowWhitespaceInTokens",
						&SeaDix::SetAllowWhitespaceInTokens),
		 InstanceMethod("clear", &SeaDix::Clear),
		 InstanceMethod("size", &SeaDix::Size),
		 InstanceMethod("empty", &SeaDix::Empty),
		 InstanceMethod("nodeCount", &SeaDix::NodeCount),
		 InstanceMethod("childrenPointersCount",
						&SeaDix::ChildrenPointersCount),
		 InstanceMethod("setNormalizationEnabled",
						&SeaDix::SetNormalizationEnabled),
		 InstanceMethod("setAllowWhitespaceInTokens",
						&SeaDix::SetAllowWhitespaceInTokens)});

	constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

	exports.Set("SeaDix", func);
	return exports;
}

SeaDix::SeaDix(const Napi::CallbackInfo &info)
	: Napi::ObjectWrap<SeaDix>(info) {
	trie = std::unique_ptr<RadixTrie>(new RadixTrie());
}

SeaDix::~SeaDix() {}

Napi::Value SeaDix::Insert(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Boolean::New(env, false);
	std::string word = info[0].As<Napi::String>().Utf8Value();
	return Napi::Boolean::New(env, trie->insert(word));
}

Napi::Value SeaDix::Search(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Boolean::New(env, false);
	std::string word = info[0].As<Napi::String>().Utf8Value();
	return Napi::Boolean::New(env, trie->search(word));
}

Napi::Value SeaDix::Remove(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Boolean::New(env, false);
	std::string word = info[0].As<Napi::String>().Utf8Value();
	return Napi::Boolean::New(env, trie->remove(word));
}

Napi::Value SeaDix::StartsWith(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Boolean::New(env, false);
	std::string prefix = info[0].As<Napi::String>().Utf8Value();
	return Napi::Boolean::New(env, trie->startsWith(prefix));
}

Napi::Value SeaDix::WordsWithPrefix(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Array::New(env, 0);
	std::string prefix = info[0].As<Napi::String>().Utf8Value();
	std::vector<std::string> words = trie->getWordsWithPrefix(prefix);
	Napi::Array arr = Napi::Array::New(env, words.size());
	for (size_t i = 0; i < words.size(); ++i)
		arr.Set(i, Napi::String::New(env, words[i]));
	return arr;
}

static std::vector<std::string> toStringVector(const Napi::Env &env,
											   const Napi::Array &arr) {
	std::vector<std::string> out;
	uint32_t len = arr.Length();
	out.reserve(len);
	for (uint32_t i = 0; i < len; ++i) {
		Napi::Value v = arr.Get(i);
		if (v.IsString())
			out.push_back(v.As<Napi::String>().Utf8Value());
		else
			out.push_back("");
	}
	return out;
}

static Napi::Array toBooleanArray(Napi::Env env,
								  const std::vector<bool> &vals) {
	Napi::Array arr = Napi::Array::New(env, vals.size());
	for (size_t i = 0; i < vals.size(); ++i)
		arr.Set(i, Napi::Boolean::New(env, vals[i]));
	return arr;
}

Napi::Value SeaDix::InsertBatch(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsArray())
		return Napi::Array::New(env, 0);
	std::vector<std::string> words =
		toStringVector(env, info[0].As<Napi::Array>());
	return toBooleanArray(env, trie->batchInsert(words));
}

Napi::Value SeaDix::SearchBatch(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsArray())
		return Napi::Array::New(env, 0);
	std::vector<std::string> words =
		toStringVector(env, info[0].As<Napi::Array>());
	return toBooleanArray(env, trie->batchSearch(words));
}

Napi::Value SeaDix::RemoveBatch(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsArray())
		return Napi::Array::New(env, 0);
	std::vector<std::string> words =
		toStringVector(env, info[0].As<Napi::Array>());
	return toBooleanArray(env, trie->batchRemove(words));
}

Napi::Value SeaDix::InsertFromFile(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Array::New(env, 0);
	std::string path = info[0].As<Napi::String>().Utf8Value();
	return toBooleanArray(env, trie->insertFromFile(path));
}

Napi::Value SeaDix::RemoveFromFile(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Array::New(env, 0);
	std::string path = info[0].As<Napi::String>().Utf8Value();
	return toBooleanArray(env, trie->removeFromFile(path));
}

Napi::Value SeaDix::SearchFromFile(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsString())
		return Napi::Array::New(env, 0);
	std::string path = info[0].As<Napi::String>().Utf8Value();
	return toBooleanArray(env, trie->searchFromFile(path));
}

Napi::Value SeaDix::Clear(const Napi::CallbackInfo &info) {
	trie->clear();
	return info.Env().Undefined();
}

Napi::Value SeaDix::Size(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), static_cast<double>(trie->size()));
}

Napi::Value SeaDix::Empty(const Napi::CallbackInfo &info) {
	return Napi::Boolean::New(info.Env(), trie->empty());
}

Napi::Value SeaDix::NodeCount(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(),
							 static_cast<double>(trie->nodeCount()));
}

Napi::Value SeaDix::ChildrenPointersCount(const Napi::CallbackInfo &info) {
	return Napi::Number::New(
		info.Env(), static_cast<double>(trie->childrenPointersCount()));
}

Napi::Value SeaDix::SetNormalizationEnabled(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsBoolean())
		return env.Undefined();
	bool enabled = info[0].As<Napi::Boolean>().Value();
	trie->setNormalizationEnabled(enabled);
	return env.Undefined();
}

Napi::Value SeaDix::SetAllowWhitespaceInTokens(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsBoolean())
		return env.Undefined();
	bool enabled = info[0].As<Napi::Boolean>().Value();
	trie->setAllowWhitespaceInTokens(enabled);
	return env.Undefined();
}

Napi::Value SeaDix::SetNormalization(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsBoolean())
		return env.Undefined();
	bool enabled = info[0].As<Napi::Boolean>().Value();
	trie->setNormalizationEnabled(enabled);
	return env.Undefined();
}

Napi::Value SeaDix::SetAllowWhitespaceInTokens(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	if (info.Length() < 1 || !info[0].IsBoolean())
		return env.Undefined();
	bool enabled = info[0].As<Napi::Boolean>().Value();
	trie->setAllowWhitespaceInTokens(enabled);
	return env.Undefined();
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
	return SeaDix::Init(env, exports);
}

NODE_API_MODULE(seadix, InitAll)
