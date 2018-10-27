from setuptools import setup, Extension

with open('README.md', 'r') as f:
  long_description = f.read()

meow_ext = Extension(
    'meowhash.cpython',
    # define_macros=[('MEOW_HASH_256', '0'), ('MEOW_HASH_512', '0')],
    sources=['meowhash/cpython.c'],
    extra_compile_args=['-mavx512f', '-mavx512vl', '-maes',
                        '-mavx512f', '-mavx512pf', '-mavx512er', '-mavx512cd',
                        '-mavx512vl', '-mavx512bw', '-mavx512dq', '-mavx512ifma',
                        '-mavx512vbmi'
                        ],
    include_dirs=['lib'])

setup(
    name='meowhash',
    version='0.1',
    description='This is a demo package',
    author='James Liu',
    author_email='contact@jamessliu.com',
    license='MIT',
    url='https://github.com/james7132/py-meowhash',
    long_description=long_description,
    packages=['meowhash'],
    classifiers=[
        "Programming Language :: Python",
        "Programming Language :: Python :: Implementation :: CPython",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.0",
        "Programming Language :: Python :: 3.1",
        "Programming Language :: Python :: 3.2",
        "Programming Language :: Python :: 3.3",
        "Programming Language :: Python :: 3.4",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
    ],
    ext_modules=[meow_ext])
