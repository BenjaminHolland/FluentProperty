#include <functional>
#include <tuple>
#include <type_traits>
#include <iostream>
#include <unordered_set>;
#include <unordered_map>;
#include <string>
#include <mutex>
using namespace std;
template <typename T,typename R>
class IFluentProperty {
public:
	typedef function<T(void)> void_generator_t;
	typedef function<T(T const&)> value_mutator_t;
	typedef function<void(T&)> value_manipulator_t;
	virtual T& get() = 0;
	virtual const T& get() const = 0;
	virtual R& set(T value) = 0;
	virtual R& set(void_generator_t generator) = 0;
	virtual R& set(value_mutator_t mutator) = 0;
	virtual R& use(value_manipulator_t manipulator) = 0;
	
	virtual R& operator<<(void_generator_t generator) {
		return set(generator);
	}
	virtual R& operator<<(value_mutator_t manipulator) {
		return set(manipulator);
	}
	virtual R& operator<<(T value) {
		return set(value);
	}
	virtual R& operator>>(T& target) {
		target = get();
		return static_cast<R&>(*this);
	}
	virtual const R& operator>>(T& target) const {
		target = get();
		return static_cast<const R&>(*this);
	}
	virtual R& operator>>(value_manipulator_t manipulator) {
		return use(manipulator);
	}
};

template <typename T>
class BasicFluentProperty:public IFluentProperty<T,BasicFluentProperty<T>> {
private:
	T _value;
public:
	virtual T& get() override{
		return _value;
	}
	virtual const T& get() const override {
		return _value;
	};
	virtual BasicFluentProperty& set(T value) {
		_value = value;
		return *this;
	}
	virtual BasicFluentProperty& set(void_generator_t generator) {
		_value = generator();
		return *this;
	};
	virtual BasicFluentProperty& set(value_mutator_t mutator) {
		_value = mutator(_value);
		return *this;
	};
	virtual BasicFluentProperty& use(value_manipulator_t manipulator) {
		manipulator(_value);
		return *this;
	};
};
template <typename T>
class SyncedFluentProperty :public IFluentProperty<T, SyncedFluentProperty<T>> {
private:
	mutex _sync;
	T _value;
public:
	virtual T& get() override {
		lock_guard<mutex> lock(_sync);
		return _value;
	}
	virtual const T& get() const override {

		lock_guard<mutex> lock(_sync);
		return _value;
	};
	virtual SycnedFluentProperty& set(T value) {

		lock_guard<mutex> lock(_sync);
		_value = value;
		return *this;
	}
	virtual SycnedFluentProperty& set(void_generator_t generator) {

		lock_guard<mutex> lock(_sync);
		_value = generator();
		return *this;
	};
	virtual SycnedFluentProperty& set(value_mutator_t mutator) {

		lock_guard<mutex> lock(_sync);
		_value = mutator(_value);
		return *this;
	};
	virtual SycnedFluentProperty& use(value_manipulator_t manipulator) {

		lock_guard<mutex> lock(_sync);
		manipulator(_value);
		return *this;
	};
};
template <typename K,typename T, typename R>
class IFluentMap {
public:
	struct destroyer_t {} erase;
	
	typedef function<T(void)> void_generator_t;
	typedef function<T(K)> key_generator_t;
	typedef function<T(T const&)> value_mutator_t;
	typedef function<T(K,T const&)> keyvalue_mutator_t;
	typedef function<T(tuple<K, T const&>)> tuple_mutator_t;
	typedef function<void(T&)> value_manipulator_t;
	typedef function<void(K, T&)> keyvalue_manipulator_t;
	typedef function<void(tuple<K, T&>)> tuple_manipulator_t;
	
	typedef tuple<K, T> set_direct_op;
	typedef tuple<K, void_generator_t> set_generate_op;
	typedef tuple<K, key_generator_t> set_kgenerate_op;
	typedef tuple<K, value_mutator_t> set_mutate_op;
	typedef tuple<K, keyvalue_mutator_t> set_kmutate_op;
	typedef tuple<K, tuple_mutator_t> set_tmutate_op;
	typedef tuple<K, value_manipulator_t> use_value_op;
	typedef tuple<K, keyvalue_manipulator_t> use_keyvalue_op;
	typedef tuple<K, tuple_manipulator_t> use_tuple_op;
	typedef tuple<K, T&> get_value_op;
	typedef tuple<K, destroyer_t> del_op;

	virtual unordered_set<K> keys() const = 0;
	virtual T& get(K) = 0;
	virtual const T& get(K) const = 0;
	virtual R& set(K, T const&) = 0;
	virtual R& set(K, void_generator_t) = 0;
	virtual R& set(K, key_generator_t) = 0;
	virtual R& set(K, value_mutator_t) = 0;
	virtual R& set(K, keyvalue_mutator_t) = 0;
	virtual R& set(K, tuple_mutator_t) = 0;
	virtual R& use(K, value_manipulator_t) = 0;
	virtual R& use(K, keyvalue_manipulator_t) = 0;
	virtual R& use(K, tuple_manipulator_t) = 0;
	virtual R& del(K) = 0;
	virtual R& del(del_op) = 0;
	
	virtual R& operator<<(set_direct_op op) {
		return set(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator<<(set_generate_op op) {
		return set(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator<<(set_kgenerate_op op) {
		return set(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator<<(set_mutate_op op) {
		return set(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator<<(set_kmutate_op op) {
		return set(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator<<(set_tmutate_op op) {
		return set(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator>>(use_value_op op) {
		return use(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator>>(use_keyvalue_op op) {
		return use(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator>>(use_tuple_op op) {
		return use(std::get<0>(op), std::get<1>(op));
	}
	virtual R& operator>>(get_value_op op) {
		std::get<1>(op) = get(std::get<0>(op));
		return static_cast<R&>(*this);
	}
	virtual R& operator<<(del_op op) {
		return del(op);
	}



};
template <typename K,typename T>
class FluentMap :public IFluentMap<K, T, FluentMap<K, T>> {
private:
	unordered_map<K, T> _items;
public:
	virtual unordered_set<K> keys() const {
		unordered_set<K> ks;
		for (auto item : _items) {
			ks.insert(item.first);
		}
		return ks;
	}
	virtual T& get(K key) {
		return _items.at(key);
	}
	virtual const T& get(K key) const { return _items.at(key); }
	virtual FluentMap<K,T>& set(K key, void_generator_t generator) {
		_items[key] = generator();
		return *this;
	}
	virtual FluentMap<K,T>& set(K key, T const& value) {
		_items[key] = value;
		return *this;
	}
	virtual FluentMap<K, T>& set(K key, key_generator_t generator) {
		_items[key] = generator(key);
		return *this;
	}
	virtual FluentMap<K, T>& set(K key, value_mutator_t mutator) {
		_items.at(key) = mutator(_items.at(key));
		return *this;
	}
	virtual FluentMap<K, T>& set(K key, keyvalue_mutator_t mutator) {
		_items.at(key) = mutator(key, _items.at(key));
		return *this;
	}
	virtual FluentMap<K, T>& set(K key,tuple_mutator_t mutator) {
		_items.at(key) = mutator(make_tuple(key, _items.at(key)));
		return *this;
	}
	virtual FluentMap<K, T>& use(K key, value_manipulator_t manipulator) {
		manipulator(_items.at(key));
		return *this;
	}
	virtual FluentMap<K, T>& use(K key, keyvalue_manipulator_t manipulator) {
		manipulator(key,_items.at(key));
		return *this;
	}
	virtual FluentMap<K, T>& use(K key, tuple_manipulator_t manipulator) {
		manipulator(tie(key, _items.at(key)));
		return *this;
	}
	virtual FluentMap<K, T>& del(K key) {
		_items.erase(key);
		return *this;
	}
	virtual FluentMap<K, T>& del(del_op eraser) {
		_items.erase(std::get<0>(eraser));
		return *this;
	}
};
int main() {
	BasicFluentProperty<int> property;
	auto print = [](int value) {cout << value << endl; };
	auto print2 = [](string key,int value) {cout <<key<<","<< value << endl; };

	property
		.set(1)
		.use(print)
		.set([](int const& i) {return i + 1; })
		.use(print)
		.set([]() {return 3; })
		.use(print)
		.use([](int & i) {return i++; })
		.use(print)
		.set(0);

	property
		<< 1
		>> print
		<< [](int const& i) {return i + 1; }
		>> print
		<< []() {return 3; }
		>> print
		>> [](int & i) {return i++; }
		>> print
		<< 0;

	BasicFluentProperty<BasicFluentProperty<int>> weird;
	weird
		<< (BasicFluentProperty<int>()<<1)
		>> [print](BasicFluentProperty<int> prop) {prop >> print; }
		>> [](BasicFluentProperty<int> & prop) {prop << 4; }
		>> [print](BasicFluentProperty<int> prop) {prop >> print; };


	FluentMap<string,int> map;

	map
		.set("A", 0)
		.use("A", print2)
		.set("B", []() {return 1; })
		.use("B", print2)
		.set("C", [](string key) {return 2; })
		.use("C", print2)
		.set("A", [](int i) {return i + 1; })
		.use("A", print2)
		.set("B", [](string k, int i) {return i + 1; })
		.use("B", print2);

	map
		<< make_tuple("A",0)
		>> make_tuple("A",print2)
		<< make_tuple("B", []() {return 1; })
		>> make_tuple("B", print2)
		<< make_tuple("C", [](string key) {return 2; })
		>> make_tuple("C", print2)
		<< make_tuple("A", [](int i) {return i + 1; })
		>> make_tuple("A", print2)
		<< make_tuple("B", [](string k, int i) {return i + 1; })
		>> make_tuple("B", print2);

	FluentMap<string, BasicFluentProperty<int>> stranger;
	stranger
		.set("A", BasicFluentProperty<int>() << 4)
		.set("B", BasicFluentProperty<int>() << 5)
		.set("C", BasicFluentProperty<int>() <<6);
	for (auto key : stranger.keys()) {
		stranger.use(key, [key](BasicFluentProperty<int>& prop) {
			prop >> [key](int i) {cout << key << "," << i << endl; };
		});
			
	}

	cin.get();

}