#include <x86intrin.h>

#include "Python.h"
#include "meow_hash.h"

#if PY_MAJOR_VERSION >= 3
#define MODINIT_NAME PyInit_cpython
#define MOD_RETURN(mod) return mod
#else
#define MODINIT_NAME initcpython
#define MOD_RETURN(mod) return
#endif

typedef struct {
  PyObject_HEAD;
  meow_state state;
  int block_size;
} MeowHashObject;

static int meowhash_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
  MeowHashObject *self;
  self = (MeowHashObject *) type->tp_alloc(type, 0);
  if (self == NULL) {
    return -1;
  }
  int *block_size = NULL;
  unsigned long long *seed = NULL;
  if (!PyArg_ParseTuple(args, "i|K", block_size, seed)) {
    return -1;
  }

  self->block_size = *block_size;
  MeowStateInit(&self->state, *seed);
  return 0;
}


static PyObject *meowhash_hash(PyObject *args) {
  int *block_size = NULL;
  Py_buffer *block = NULL;
  unsigned long long *seed = NULL;
  if (!PyArg_ParseTuple(args, "iy*|K", block_size, block, seed)) {
    return NULL;
  }

  meow_lane lane;
  switch (*block_size) {
    case 128:
      lane = MeowHash1(*seed, block->len, block->buf);
      break;
#if MEOW_HASH_256
    case 256:
      lane = MeowHash2(*seed, block->len, block->buf);
      break;
#endif
#if MEOW_HASH_512
    case 512:
      lane = MeowHash4(*seed, block->len, block->buf);
      break;
#endif
    default:
      return NULL;
  }

  return Py_BuildValue("s#", &lane, sizeof(lane));
}

static PyObject *meowhash_update(MeowHashObject *self, PyObject *args) {
  Py_buffer *block = NULL;
  if (!PyArg_ParseTuple(args, "y*", *block)) {
    return NULL;
  }

  switch (self->block_size) {
    case 128:
      MeowHash1Update(&self->state, block->len, block->buf);
      break;
#if MEOW_HASH_256
    case 256:
      MeowHash2Update(&self->state, block->len, block->buf);
      break;
#endif
#if MEOW_HASH_512
    case 512:
      MeowHash4Update(&self->state, block->len, block->buf);
      break;
#endif
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
#if MEOW_HASH_256
    case 256:
      lane = MeowHash2Flush(self->state);
      break;
#endif
#if MEOW_HASH_512
    case 512:
      lane = MeowHash4Flush(self->state);
      break;
#endif
    default:
      return NULL;
  }
  return Py_BuildValue("s#", &lane, sizeof(lane));
}

static PyObject *meowhash_digest_size(MeowHashObject *self, PyObject *args) {
  return Py_BuildValue("i", sizeof(self->state.IV));
}

static PyObject *meowhash_block_size(MeowHashObject *self, PyObject *args) {
  return Py_BuildValue("i", sizeof(self->state.IV));
}

/**
 * MeowHash.__del__()
 */
static void meow_hash_dealloc(MeowHashObject *self) {
    PyObject_Del(self);
}

static struct PyMethodDef hash_methods[] = {
    {"update", (PyCFunction)meowhash_update, METH_VARARGS},
    {"digest", (PyCFunction)meowhash_digest, METH_NOARGS},
    {"digest_size", (PyCFunction)meowhash_digest_size, METH_NOARGS},
    {"block_size", (PyCFunction)meowhash_block_size, METH_NOARGS},
    {NULL, NULL}
};

static struct PyMethodDef module_methods[] = {
    {"hash", (PyCFunction)meowhash_hash, METH_VARARGS},
    {NULL, NULL}
};

static PyTypeObject PyMeowHash_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "MeowHash",
    .tp_basicsize = sizeof(MeowHashObject),
    .tp_doc = "Non-cryptographic hash function",
    .tp_dealloc = (destructor) meow_hash_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = hash_methods,
    .tp_new = meowhash_new
};


#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "cpython",
    NULL,
    -1,
    module_methods,
    NULL,
    NULL,
    NULL,
    NULL
};
#endif

/**
 * Construct a PyString and append it to a list, returning 0 on success or -1
 * on error.
 */
static int
append_string(PyObject *list, const char *s)
{
#if PY_MAJOR_VERSION >= 3
    PyObject *obj = PyUnicode_FromString(s);
#else
    PyObject *obj = PyBytes_FromStringAndSize(s, strlen(s));
#endif

    if(! obj) {
        return -1;
    }
    if(PyList_Append(list, obj)) {
        Py_DECREF(obj);
        return -1;
    }
    Py_DECREF(obj);
    return 0;
}

static int init_types(PyObject *mod, PyObject *__all__) {
    static PyTypeObject *types[] = {
        &PyMeowHash_Type,
        NULL
    };

    int i;
    for(i = 0; types[i]; i++) {
        PyTypeObject *type = types[i];

        if(PyType_Ready(type)) {
            PyErr_SetString(PyExc_ValueError, "Type not ready.");
            return -1;
        }
        if(PyObject_SetAttrString(mod, type->tp_name, (PyObject *)type)) {
            PyErr_SetString(PyExc_ValueError, "Cannot set on module.");
            return -1;
        }

        if(type->tp_name[0] != '_' && append_string(__all__, type->tp_name)) {
            PyErr_SetString(PyExc_ValueError, "Cannot add to __all__.");
            return -1;
        }
    }

    return 0;
}

PyMODINIT_FUNC
PyInit_cpython(void)
{
  PyObject *__all__;

#if PY_MAJOR_VERSION >= 3
   PyObject *mod = PyModule_Create(&moduledef);
#else
   PyObject *mod = Py_InitModule3("cpython", module_methods, "");
#endif

  if (mod == NULL){
    PyErr_SetString(PyExc_ValueError, "Can't create module.");
    MOD_RETURN(NULL);
  }

  if(! ((__all__ = PyList_New(0)))) {
    PyErr_SetString(PyExc_ValueError, "Cannot create __all__.");
    MOD_RETURN(NULL);
  }
  if(init_types(mod, __all__)) {
    PyErr_SetString(PyExc_ValueError, "Cannot init types.");
    MOD_RETURN(NULL);
  }

  if(PyObject_SetAttrString(mod, "__all__", __all__)) {
    PyErr_SetString(PyExc_ValueError, "Can't set __all__.");
    MOD_RETURN(NULL);
  }
  Py_DECREF(__all__);
  MOD_RETURN(mod);
}
