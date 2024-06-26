#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 16>
class ADS_set {
public:
    using value_type = Key;
    using key_type = Key;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using key_equal = std::equal_to<key_type>;
    using hasher = std::hash<key_type>;

    ADS_set();
    ADS_set(std::initializer_list<key_type> ilist);
    template<typename InputIt> ADS_set(InputIt first, InputIt last);
    ADS_set(const ADS_set &other);

   ~ADS_set();

    ADS_set &operator=(const ADS_set &other);
    ADS_set &operator=(std::initializer_list<key_type> ilist);

    size_type size() const;
    bool empty() const;

    void insert(std::initializer_list<key_type> ilist);
    std::pair<bool, bool> insert(const key_type &key);
    template<typename InputIt> void insert(InputIt first, InputIt last);

    void clear();
    size_type erase(const key_type &key);

    size_type count(const key_type &key) const;
    bool find(const key_type &key) const;

    void swap(ADS_set &other);

    void dump(std::ostream &o = std::cerr) const;

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
        if (lhs.table_size != rhs.table_size) return false;
        for (size_type i = 0; i < lhs.table_size; ++i) {
            if (lhs.buckets[i].size != rhs.buckets[i].size) return false;
            for (size_type j = 0; j < lhs.buckets[i].size; ++j) {
                if (!rhs.find(lhs.buckets[i].data[j])) return false;
            }
        }
        return true;
    }

    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
        return !(lhs == rhs);
    }

private:
    struct Bucket {
        key_type *data;
        size_type size;
        size_type capacity;

        Bucket() : data(nullptr), size(0), capacity(0) {}

        ~Bucket() { delete[] data; }

        void add(const key_type &key) {
            if (size == capacity) {
                capacity = capacity ? 2 * capacity : 1;
                key_type *new_data = new key_type[capacity];
                for (size_type i = 0; i < size; ++i) {
                    new_data[i] = data[i];
                }
                delete[] data;
                data = new_data;
            }
            data[size++] = key;
        }

        void remove(const key_type &key) {
            for (size_type i = 0; i < size; ++i) {
                if (data[i] == key) {
                    data[i] = data[--size];
                    return;
                }
            }
        }

        bool contains(const key_type &key) const {
            for (size_type i = 0; i < size; ++i) {
                if (key_equal{}(data[i],key)) return true;
            }
            return false;
        }
    };

    size_type table_size;
    size_type num_elements;
    Bucket *buckets;

    void rehash();
    size_type bucket_index(const key_type &key) const {
        return hasher{}(key) % table_size;
    }
};

template <typename Key, size_t N>
ADS_set<Key, N>::ADS_set() : table_size(N), num_elements(0), buckets(new Bucket[N]) {}

template <typename Key, size_t N>
ADS_set<Key, N>::ADS_set(std::initializer_list<key_type> ilist) : ADS_set() {
    insert(ilist.begin(), ilist.end());
}

template <typename Key, size_t N>
template<typename InputIt>
ADS_set<Key, N>::ADS_set(InputIt first, InputIt last) : ADS_set() {
    insert(first, last);
}

template <typename Key, size_t N>
ADS_set<Key, N>::ADS_set(const ADS_set &other) : table_size(other.table_size), num_elements(other.num_elements), buckets(new Bucket[other.table_size]) {
    for (size_type i = 0; i < table_size; ++i) {
        buckets[i].size = other.buckets[i].size;
        buckets[i].capacity = other.buckets[i].capacity;
        if (buckets[i].capacity) {
            buckets[i].data = new key_type[buckets[i].capacity];
            for (size_type j = 0; j < buckets[i].size; ++j) {
                buckets[i].data[j] = other.buckets[i].data[j];
            }
        }
    }
}

template <typename Key, size_t N>
ADS_set<Key, N>::~ADS_set() {
    delete[] buckets;
}

template <typename Key, size_t N>
ADS_set<Key, N> &ADS_set<Key, N>::operator=(const ADS_set &other) {
    if (this == &other) return *this;
    ADS_set tmp(other);
    swap(tmp);
    return *this;
}

template <typename Key, size_t N>
ADS_set<Key, N> &ADS_set<Key, N>::operator=(std::initializer_list<key_type> ilist) {
    clear();
    insert(ilist);
    return *this;
}

template <typename Key, size_t N>
typename ADS_set<Key, N>::size_type ADS_set<Key, N>::size() const {
    return num_elements;
}

template <typename Key, size_t N>
bool ADS_set<Key, N>::empty() const {
    return num_elements == 0;
}

template <typename Key, size_t N>
void ADS_set<Key, N>::insert(std::initializer_list<key_type> ilist) {
    for (const auto &key : ilist) {
        insert(key);
    }
}

template <typename Key, size_t N>
std::pair<bool, bool> ADS_set<Key, N>::insert(const key_type &key) {
    size_type index = bucket_index(key);
    if (buckets[index].contains(key)) return {false, false};
    buckets[index].add(key);
    ++num_elements;
    if (num_elements > table_size * 2) rehash();
    return {true, true};
}

template <typename Key, size_t N>
template<typename InputIt>
void ADS_set<Key, N>::insert(InputIt first, InputIt last) {
    for (InputIt it = first; it != last; ++it) {
        insert(*it);
    }
}

template <typename Key, size_t N>
void ADS_set<Key, N>::clear() {
    delete[] buckets;
    buckets = new Bucket[table_size];
    num_elements = 0;
}

template <typename Key, size_t N>
typename ADS_set<Key, N>::size_type ADS_set<Key, N>::erase(const key_type &key) {
    size_type index = bucket_index(key);
    if (!buckets[index].contains(key)) return 0;
    buckets[index].remove(key);
    --num_elements;
    return 1;
}

template <typename Key, size_t N>
typename ADS_set<Key, N>::size_type ADS_set<Key, N>::count(const key_type &key) const {
    return buckets[bucket_index(key)].contains(key) ? 1 : 0;
}

template <typename Key, size_t N>
bool ADS_set<Key, N>::find(const key_type &key) const {
    return buckets[bucket_index(key)].contains(key);
}

template <typename Key, size_t N>
void ADS_set<Key, N>::swap(ADS_set &other) {
    std::swap(table_size, other.table_size);
    std::swap(num_elements, other.num_elements);
    std::swap(buckets, other.buckets);
}

template <typename Key, size_t N>
void ADS_set<Key, N>::dump(std::ostream &o) const {
    for (size_type i = 0; i < table_size; ++i) {
        o << "Bucket " << i << ": ";
        for (size_type j = 0; j < buckets[i].size; ++j) {
            o << buckets[i].data[j] << " ";
        }
        o << "\n";
    }
}

template <typename Key, size_t N>
void ADS_set<Key, N>::rehash() {
    size_type new_table_size = table_size * 2;
    Bucket *new_buckets = new Bucket[new_table_size];
    for (size_type i = 0; i < table_size; ++i) {
        for (size_type j = 0; j < buckets[i].size; ++j) {
          size_type new_index = hasher{}(buckets[i].data[j]) % new_table_size;
          new_buckets[new_index].add(buckets[i].data[j]);
        }
    }
    delete[] buckets;
    buckets = new_buckets;
    table_size = new_table_size;
}

#endif // ADS_SET_H

