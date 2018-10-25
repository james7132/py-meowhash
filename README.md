# py-meowhash

py-meowhash is a simple python binding for
[cmuratori/meow_hash](https://github.com/cmuratori/meow_hash), an extremely fast
non-cryptographic hash function. Meant to act as a drop in replacement for any
`hashlib` hashing algorithm.

**NOTE**: meow_hash, and by proxy py-meowhash are considered experimental or
alpha quality software and should be used as such.

## Usage

Can be used as a drop in replacement for `hashlib` algorithms:

```python
hash = meowhash.new()
hash.update(b'hello world')
d = hash.digest()
h = hash.hexdigest()
hash2 = hash.copy()

# Available attributes
hash.name
hash.digest_size
hash.hash_size
```

## Caveats and Known Issues

This library currently is not portable and requires AVX instruction support on
the local machine to run properly. The library does not check for this support
as of right now and may segfault on calling

Please check the
[meow_hash issue tracker](https://github.com/cmuratori/meow_hash/issues) for
for other issues that one may encounter.
