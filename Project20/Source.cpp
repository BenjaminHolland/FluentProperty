#include <functional>
#include <optional>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <tuple>
using namespace std;
template <typename T>
class IProperty {
public:
	virtual T& get() = 0;
	virtual const T& get() const = 0;
	virtual IProperty<T>& set(T) = 0;
	virtual IProperty<T>& set(function<T(void)>) = 0;
	virtual IProperty<T>& set(function<T(T const&)>) = 0;
	virtual IProperty<T>& use(function<void(T&)>) = 0;
	virtual IProperty<T>& operator<<(T value) {
		return set(value);
	}
	virtual IProperty<T>& operator<<(function <T(void)> make) {
		return set(make);
	}
	virtual IProperty<T>& operator<<(function<T(T const&)> make) {
		return set(make);
	}
	virtual IProperty<T>& operator>>(T& target) {
		target = get();
		return *this;
	}
	virtual IProperty<T>& operator>>(function<void(T&)> execute) {
		return use(execute);
	}
};
template <typename T>
class BasicProperty :public IProperty<T> {
private:
	T _value;
public:
	T& get() override {
		return _value;
	}
	const T& get() const override {
		return _value;
	}
	IProperty<T>& set(T value) override {
		_value = value;
		return *this;
	}
	IProperty<T>& set(function<T(void)> make) override {
		_value = make();
		return *this;
	}
	IProperty<T>& set(function<T(T const&)> make) override {
		_value = make(_value);
		return *this;
	}
	IProperty<T>& use(function<void(T&)> execute) override {
		execute(_value);
		return *this;
	}
};
template <typename T>
class SyncedProperty :public IProperty<T> {
private:
	mutable mutex _lock;
	T _value;
public:
	T& get() override {
		lock_guard<mutex> lock(_lock);
		return _value;
	}
	const T& get() const override {
		lock_guard<mutex> lock(_lock);
		return _value;
	}
	IProperty<T>& set(T value) override {
		lock_guard<mutex> lock(_lock);
		_value = value;
		return *this;
	}
	IProperty<T>& set(function<T(void)> make) override {
		lock_guard<mutex> lock(_lock);
		_value = make();
		return *this;
	}
	IProperty<T>& set(function<T(T const&)> make) override {
		lock_guard<mutex> lock(_lock);
		_value = make(_value);
		return *this;
	}
	IProperty<T>& use(function<void(T&)> execute) override {
		lock_guard<mutex> lock(_lock);
		execute(_value);
		return *this;
	}
};

template <typename K, typename T>
class IIndexedProperty {
public:
	virtual T& get(K const&) = 0;
	virtual const T& get(K const&) const = 0;
	virtual unordered_set<K> keys() const = 0;
	virtual IIndexedProperty<K, T>& set(K const&, T) = 0;

	virtual IIndexedProperty<K, T>& set(K const&, function<T(K const&, T const&)>) = 0;
	virtual IIndexedProperty<K, T>& set(K const&, function<T(K const&)>) = 0;
	virtual IIndexedProperty<K, T>& set(K const&, function<T(T const&)>) = 0;
	virtual IIndexedProperty<K, T>& set(K const&, function<T(void)>) = 0;

	virtual IIndexedProperty<K, T>& use(K const&, function<void(K const&, T&)>) = 0;
	virtual IIndexedProperty<K, T>& use(K const&, function<void(T&)>) = 0;

	virtual IIndexedProperty<K, T>& del(K const&) = 0;

	virtual IIndexedProperty<K, T>& operator<<(pair<K, T> record) {
		return set(record.first, record.second);
	}
	virtual IIndexedProperty<K, T>& operator<<(pair<K, function<T(K const&, T const&)>>& recordGen) {
		return set(recordGen.first, recordGen.second);
	}
	virtual IIndexedProperty<K, T>& operator<<(pair<K, function<T(K const&)>> recordGen) {
		return set(recordGen.first, recordGen.second);
	}
	virtual IIndexedProperty<K, T>& operator<<(pair<K, function<T(T const&)>> recordGen) {
		return set(recordGen.first, recordGen.second);
	}
	virtual IIndexedProperty<K, T>& operator<<(pair<K, function<T(void)>> recordGen) {
		return set(recordGen.first, recordGen.second);
	}
	virtual IIndexedProperty<K, T>& operator>>(pair<K, function<void(K const&, T&)>> execute) {
		return use(execute.first, execute.second);
	}
	virtual IIndexedProperty<K, T>& operator>>(pair<K, function<void(T&)>> execute) {
		return use(execute.first, execute.second);
	}
	virtual IIndexedProperty<K, T>& operator>>(pair<K, T&> query) {
		query.second = get(query.first);
		return *this;
	}
	virtual IIndexedProperty<K, T>& operator>>(tuple<K, T&> query) {
		std::get<1>(query) = get(std::get<0>(query));
		return *this;
	}
};
template<typename K, typename T>
class MappedProperty :public IIndexedProperty<K, T> {
private:
	unordered_map<K, T> _items;
public:


	virtual unordered_set<K> keys() const override {
		unordered_set<K> ks;
		for (auto record : _items) {
			ks.insert(record.first);
		}
		return ks;
	}
	virtual T& get(K const& key) override {
		return _items.at(key);
	}
	virtual const T& get(K const& key) const override {
		return _items.at(key);
	}
	virtual IIndexedProperty<K, T>& set(K const& key, function<T(K const&, T const&)> generate)override {
		_items.at(key) = generate(key, _items.at(key));
		return *this;
	}
	virtual IIndexedProperty<K, T>& set(K const& key, function<T(K const&)> generate) override {
		_items[key] = generate(key);
		return *this;
	}
	virtual IIndexedProperty<K, T>& set(K const& key, function<T(T const&)> generate) override {
		_items.at(key) = generate(_items.at(key));
		return *this;
	}
	virtual IIndexedProperty<K, T>& set(K const& key, function<T(void)> generate) override {
		_items[key] = generate();
		return *this;
	}
	virtual IIndexedProperty<K, T>& set(K const& key, T value) override {
		_items[key] = value;
		return *this;
	}

	virtual IIndexedProperty<K, T>& use(K const& key, function<void(K const&, T&)> execute) override {
		execute(key, _items.at(key));
		return *this;
	}
	virtual IIndexedProperty<K, T>& use(K const& key, function<void(T&)> execute)override {
		execute(_items.at(key));
		return *this;
	}
	virtual IIndexedProperty<K, T>& del(K const& key) override {
		_items.erase(key);
		return *this;
	}
};
int main() {
	MappedProperty<string, int> p;
	auto weird_gen = [](string key) {
		if (key == "D") return 4;
		if (key == "E") return 5;
		return 6;
	};
	auto auto_gen = [](string key) {
		static int i = 7;
		int ret = i;
		i++;
		return ret;
	};
	auto auto_key = []() {
		static int i = 0;
		char t[] = { (char)(i + 0x61),'\0' };
		string ret(t);
		i++;
		return ret;
	};
	p
		.set("A", 1)
		.set("B", 2)
		.set("C", []() {return 3; })
		.set("D", weird_gen)
		.set("E", weird_gen)
		.set("F", weird_gen)
		.set("G", auto_gen)
		.set("H", auto_gen)
		.set("I", auto_gen);
	auto auto_elem = [auto_key, auto_gen]() {
		return make_pair(auto_key(), auto_gen);
	};
	for (int i = 0; i < 26; i++) {
		p << auto_elem();
	}

	for (auto key : p.keys()) {
		int x;
		p >> tie(key, x);
		cout << key << "," << x << endl;
	}
	cin.get();
	return 0;
}