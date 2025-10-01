#ifndef SEADIX_H
#define SEADIX_H

#include "RadixTrie.h"
#include <napi.h>

class SeaDix : public Napi::ObjectWrap<SeaDix> {
  public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);
	SeaDix(const Napi::CallbackInfo &info);
	~SeaDix();

  private:
	// Core
	Napi::Value Insert(const Napi::CallbackInfo &info);
	Napi::Value Search(const Napi::CallbackInfo &info);
	Napi::Value Remove(const Napi::CallbackInfo &info);
	Napi::Value StartsWith(const Napi::CallbackInfo &info);
	Napi::Value WordsWithPrefix(const Napi::CallbackInfo &info);

	// Batch
	Napi::Value InsertBatch(const Napi::CallbackInfo &info);
	Napi::Value SearchBatch(const Napi::CallbackInfo &info);
	Napi::Value RemoveBatch(const Napi::CallbackInfo &info);

	// File
	Napi::Value InsertFromFile(const Napi::CallbackInfo &info);
	Napi::Value RemoveFromFile(const Napi::CallbackInfo &info);
	Napi::Value SearchFromFile(const Napi::CallbackInfo &info);

	// Config
	Napi::Value SetNormalization(const Napi::CallbackInfo &info);
	Napi::Value SetAllowWhitespaceInTokens(const Napi::CallbackInfo &info);

	// Utils
	Napi::Value Clear(const Napi::CallbackInfo &info);
	Napi::Value Size(const Napi::CallbackInfo &info);
	Napi::Value Empty(const Napi::CallbackInfo &info);
	Napi::Value NodeCount(const Napi::CallbackInfo &info);
	Napi::Value ChildrenPointersCount(const Napi::CallbackInfo &info);

	// Config
	Napi::Value SetNormalizationEnabled(const Napi::CallbackInfo &info);
	Napi::Value SetAllowWhitespaceInTokens(const Napi::CallbackInfo &info);

	std::unique_ptr<RadixTrie> trie;
	static Napi::FunctionReference constructor;
};

#endif // SEADIX_H
