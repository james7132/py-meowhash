#include "Python.h"

#include <intrin.h>

#include "meow_hash.h"

typedef struct {
  PyObject_HEAD;
  meow_state state;
  int block_size;
} MeowHashObject;

static PyObject *meowhash_new(PyTypeObject *type, PyObject *args,
                              PyObject *kwds) {
  int *block_size;
  unsigned long long *seed;
  if (!PyArg_ParseTuple(args, "i|K", block_size, seed)) {
    return NULL;
  }

  MeowHashObject *self;
  OBJECT_INIT(self);
  self->block_size = *block_size;
  MeowStateInit(&self->state, *seed);
  return (PyObject *)self;
}

static PyObject *meowhash_hash(PyObject *args) {
  int *block_size;
  Py_buffer *block;
  unsigned long long seed;
  if (!PyArg_ParseTuple(args, "iy*|K", block_size, block, seed)) {
    return NULL;
  }

  meow_lane lane;
  switch (*block_size) {
    case 128:
      lane = MeowHash1(seed, block->len, block->buf);
      break;
    case 256:
      lane = MeowHash2(seed, block->len, block->buf);
      break;
    case 512:
      lane = MeowHash4(seed, block->len, block->buf);
      break;
    default:
      return NULL;
  }

  return Py_BuildValue("s#", &lane, sizeof(lane))
}

static PyObject *meowhash_update(MeowHashObject *self, PyObject *args) {
  Py_buffer *block;
  if (!PyArg_ParseTuple(args, "y*", *block)) {
    return NULL;
  }

  switch (self->block_size) {
    case 128:
      MeowHash1Update(&self->state, block->len, block->buf);
      break;
    case 256:
      MeowHash2Update(&self->state, block->len, block->buf);
      break;
    case 512:
      MeowHash4Update(&self->state, block->len, block->buf);
      break;
    default:
      return NULL;
  }

  return Py_None;
}

static PyObject *meowhash_digest(MeowHashObject *self, PyObject *args) {
  meow_lane lane;
  switch (self->block_size) {
    case 128:
      lane = MeowHash1Flush(self->state);
      break;
    case 256:
      lane = MeowHash2Flush(self->state);
      break;
    case 512:
      lane = MeowHash4Flush(self->state);
      break;
    default:
      return NULL;
  }
  return Py_BuildValue("s#", &lane, sizeof(lane))
}

static PyObject *meowhash_digest_size(MeowHashObject *self, PyObject *args) {
  return Py_BuildValue("i", sizeof(self->state.IV));
}

static PyObject *meowhash_block_size(MeowHashObject *self, PyObject *args) {
  return Py_BuildValue("i", sizeof(self->state.IV));
}
