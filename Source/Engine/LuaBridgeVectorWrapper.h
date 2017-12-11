#ifndef LENGINE_LB_VEC_WRAPPER
#define LENGINE_LB_VEC_WRAPPER

#include <vector>

/**
 * This class is needed because of the issue found here:
 * https://github.com/vinniefalco/LuaBridge/issues/123
 */
template <class T>
class LB_VEC_WRAPPER {
 public:
  LB_VEC_WRAPPER() {}
  LB_VEC_WRAPPER(const std::vector<T>* initialValues) {
    for (auto i = initialValues->begin(); i != initialValues->end(); i++) {
      mWrappedVector.push_back(*i);
    }
  }
  ~LB_VEC_WRAPPER() {}

  void Push(T value) { mWrappedVector.push_back(value); }
  T At(int index) { return mWrappedVector.at(index); }
  int Size() { return mWrappedVector.size(); }
  bool Empty() { return mWrappedVector.empty(); }

 private:
  std::vector<T> mWrappedVector;
};

#endif