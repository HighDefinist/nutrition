// This code is modified, but originally from the "The CUDA Handbook". It was published with the 2-clause BSD license.

#pragma once

//#include "mz/base.h"
#include <iterator>
#include <type_traits>
#ifndef __PUalso
#  define __PUalso 
#endif

namespace util {
  namespace lang {
    namespace detail {

      template <typename T>
      struct range_iter_base: std::iterator<std::input_iterator_tag,T> {
        __PUalso range_iter_base(T current_): current(current_) {}

        __PUalso T operator *() const { return current; }

        __PUalso T const* operator ->() const { return &current; }

        __PUalso range_iter_base& operator ++() {
          ++current;
          return *this;
        }

        __PUalso range_iter_base operator ++(int) {
          auto copy = *this;
          ++*this;
          return copy;
        }

        __PUalso bool operator ==(range_iter_base const& other) const {
          return current==other.current;
        }

        __PUalso bool operator !=(range_iter_base const& other) const {
          return !(*this==other);
        }

      protected:
        T current;
      };

    } // namespace detail

    template <typename T>
    struct range_proxy {
      struct iter: detail::range_iter_base<T> {
        __PUalso iter(T current_): detail::range_iter_base<T>(current_) {}
      };

      struct step_range_proxy {
        struct iter: detail::range_iter_base<T> {
          __PUalso iter(T current_,T step_)
            : detail::range_iter_base<T>(current_),step(step_) {}

          using detail::range_iter_base<T>::current;

          __PUalso iter& operator ++() {
            current += step;
            return *this;
          }

          __PUalso iter operator ++(int) {
            auto copy = *this;
            ++*this;
            return copy;
          }

          // Loses commutativity. Iterator-based ranges are simply broken. :-(
          __PUalso bool operator ==(iter const& other) const {
            return step>0 ? current>=other.current
              : current<other.current;
          }

          __PUalso bool operator !=(iter const& other) const {
            return not (*this==other);
          }

        private:
          T step;
        };

        __PUalso step_range_proxy(T begin,T end,T step)
          : begin_(begin,step),end_(end,step) {}

        __PUalso iter begin() const { return begin_; }

        __PUalso iter end() const { return end_; }

      private:
        iter begin_;
        iter end_;
      };

      __PUalso range_proxy(T begin,T end): begin_(begin),end_(end) {}

      __PUalso step_range_proxy step(T step) {
        return{*begin_, *end_, step};
      }

      __PUalso iter begin() const { return begin_; }

      __PUalso iter end() const { return end_; }

    private:
      iter begin_;
      iter end_;
    };

    template <typename T>
    struct infinite_range_proxy {
      struct iter: detail::range_iter_base<T> {
        __PUalso iter(T current = T()): detail::range_iter_base<T>(current) {}

        __PUalso bool operator ==(iter const&) const { return false; }

        __PUalso bool operator !=(iter const&) const { return true; }
      };

      struct step_range_proxy {
        struct iter: detail::range_iter_base<T> {
          __PUalso iter(T current_ = T(),T step_ = T())
            : detail::range_iter_base<T>(current_),step(step_) {}

          using detail::range_iter_base<T>::current;

          __PUalso iter& operator ++() {
            current += step;
            return *this;
          }

          __PUalso iter operator ++(int) {
            auto copy = *this;
            ++*this;
            return copy;
          }

          __PUalso bool operator ==(iter const&) const { return false; }

          __PUalso bool operator !=(iter const&) const { return true; }

        private:
          T step;
        };

        __PUalso step_range_proxy(T begin,T step): begin_(begin,step) {}

        __PUalso iter begin() const { return begin_; }

        __PUalso iter end() const { return  iter(); }

      private:
        iter begin_;
      };

      __PUalso infinite_range_proxy(T begin): begin_(begin) {}

      __PUalso step_range_proxy step(T step) {
        return step_range_proxy(*begin_,step);
      }

      __PUalso iter begin() const { return begin_; }

      __PUalso iter end() const { return iter(); }

    private:
      iter begin_;
    };

    template <typename T>
    __PUalso range_proxy<T> range(T begin,T end) {
      return{begin, end};
    }

    template <typename T>
    __PUalso range_proxy<T> range(T end) {
      return{static_cast<T>(0), end};
    }

    //template <typename T>
    //infinite_range_proxy<T> range(T begin) {
    //    return {begin};
    //}

    namespace traits {

      template <typename C>
      struct has_size {
        template <typename T>
        static constexpr auto check(T*) ->
          typename std::is_integral<decltype(std::declval<T const>().size())>::type;

        template <typename>
        static constexpr auto check(...)->std::false_type;

        using type = decltype(check<C>(0));
        static constexpr bool value = type::value;
      };

    } // namespace traits

    template <typename C,typename = typename std::enable_if<traits::has_size<C>::value>> 
    auto indices(C const& cont) -> range_proxy<decltype(cont.size())> {
      return{0, cont.size()};
    }

    template <typename T,std::size_t N> 
    __PUalso range_proxy<std::size_t> indices(T(&)[N]) {
      return{0, N};
    }

    template <typename T> 
    __PUalso range_proxy<typename std::initializer_list<T>::size_type> indices(std::initializer_list<T>&& cont) {
      return{0, cont.size()};
    }

  }
} // namespace util::lang


using util::lang::indices;
using util::lang::range;
