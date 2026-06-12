# Swisse Table
A swisse table is a fast, efficient, and cache-friendly hash table implementation in C++. It is designed to provide high performance for a wide range of applications, including those that require frequent insertions and deletions.



## Testing

To run tests, you will have to install gtest. You can do this by running the following command in your terminal:

```bash
# Ubuntu/Debian
sudo apt-get install libgtest-dev

# macOS (using Homebrew)
brew install googletest
```

## Benchmarking
For benchmarking we compare our `mischa::swiss_map<K, V>` against the `std::unordered_map<K, V>`, `boost::unordered_map<K, V>`, and the implementation that this project is based off of `absl::flat_hash_map<K, V>`.

This means we need to install the two external libraries of `boost` and `absl`. If you don't already have these installed, run the following commands in your terminal:
```bash
sudo apt-get install libboost-dev
sudo apt-get install libabsl-dev
```
