#ifndef _SIGC_FUNCTORS_SLOT_H_
#define _SIGC_FUNCTORS_SLOT_H_

#include <sigc++/trackable.h>
#include <sigc++/visit_each.h>
#include <sigc++/adaptors/adaptor_trait.h>
#include <sigc++/functors/slot_base.h>

namespace sigc {

namespace internal {

/** A typed slot_rep.
 * A typed slot_rep holds a functor that can be invoked from
 * slot::operator()(). visit_each() is used to visit the functor's
 * targets that inherit trackable recursively and register the
 * notification callback. Consequently the slot_rep object will be
 * notified when some referred object is destroyed or overwritten.
 */
template <class T_functor>
struct typed_slot_rep : public slot_rep
{
private:
  using self = typed_slot_rep<T_functor>;

public:
  /* Use an adaptor type so that arguments can be passed as const references
   * through explicit template instantiation from slot_call#::call_it() */
  using adaptor_type = typename adaptor_trait<T_functor>::adaptor_type;

  /** The functor contained by this slot_rep object. */
  adaptor_type functor_;

  /** Constructs an invalid typed slot_rep object.
   * The notification callback is registered using visit_each().
   * @param functor The functor contained by the new slot_rep object.
   */
  inline typed_slot_rep(const T_functor& functor)
    : slot_rep(nullptr, &destroy, &dup), functor_(functor)
    { sigc::visit_each_type<trackable*>(slot_do_bind(this), functor_); }

  inline typed_slot_rep(const typed_slot_rep& cl)
    : slot_rep(cl.call_, &destroy, &dup), functor_(cl.functor_)
    { sigc::visit_each_type<trackable*>(slot_do_bind(this), functor_); }

  typed_slot_rep& operator=(const typed_slot_rep& src) = delete;

  typed_slot_rep(typed_slot_rep&& src) = delete;
  typed_slot_rep& operator=(typed_slot_rep&& src) = delete;

  inline ~typed_slot_rep()
    {
      call_ = nullptr;
      destroy_ = nullptr;
      sigc::visit_each_type<trackable*>(slot_do_unbind(this), functor_);
    }

private:
  /** Detaches the stored functor from the other referred trackables and destroys it.
   * This does not destroy the base slot_rep object.
   */
  static void destroy(notifiable* data)
    {
      self* self_ = static_cast<self*>(reinterpret_cast<slot_rep*>(data));
      self_->call_ = nullptr;
      self_->destroy_ = nullptr;
      sigc::visit_each_type<trackable*>(slot_do_unbind(self_), self_->functor_);
      self_->functor_.~adaptor_type();
      /* don't call disconnect() here: destroy() is either called
       * a) from the parent itself (in which case disconnect() leads to a segfault) or
       * b) from a parentless slot (in which case disconnect() does nothing)
       */
    }

  /** Makes a deep copy of the slot_rep object.
   * Deep copy means that the notification callback of the new
   * slot_rep object is registered in the referred trackables.
   * @return A deep copy of the slot_rep object.
   */
  static slot_rep* dup(slot_rep* a_rep)
    {
      return new self(*static_cast<self*>(a_rep));
    }
};


/** Abstracts functor execution.
 * call_it() invokes a functor of type @e T_functor with a list of
 * parameters whose types are given by the template arguments.
 * address() forms a function pointer from call_it().
 *
 * The following template arguments are used:
 * - @e T_functor The functor type.
 * - @e T_return The return type of call_it().
 * - @e T_arg Argument types used in the definition of call_it().
 *
 */
template<class T_functor, class T_return, class... T_arg>
struct slot_call
{
  /** Invokes a functor of type @p T_functor.
   * @param rep slot_rep object that holds a functor of type @p T_functor.
   * @param _A_a Arguments to be passed on to the functor.
   * @return The return values of the functor invocation.
   */
  static T_return call_it(slot_rep* rep, type_trait_take_t<T_arg>... a_)
    {
      using typed_slot = typed_slot_rep<T_functor>;
      typed_slot *typed_rep = static_cast<typed_slot*>(rep);
      return (typed_rep->functor_).template operator()<type_trait_take_t<T_arg>...>
               (a_...);
    }

  /** Forms a function pointer from call_it().
   * @return A function pointer formed from call_it().
   */
  static hook address()
    { return reinterpret_cast<hook>(&call_it); }
};


} /* namespace internal */


// Because slot is opaque, visit_each() will not visit its internal members.
// Those members are not reachable by visit_each() after the slot has been
// constructed. But when a slot contains another slot, the outer slot will become
// the parent of the inner slot, with similar results. See the description of
// slot's specialization of the visitor struct.

/** Converts an arbitrary functor to a unified type which is opaque.
 * sigc::slot itself is a functor or to be more precise a closure. It contains
 * a single, arbitrary functor (or closure) that is executed in operator()().
 *
 * The template arguments determine the function signature of operator()():
 * - @e T_return The return type of operator()(). * - @e T_arg Argument types used in the definition of operator()().
 *
 * To use simply assign the desired functor to the slot. If the functor
 * is not compatible with the parameter list defined with the template
 * arguments compiler errors are triggered. When called the slot
 * will invoke the functor with minimal copies.
 * block() and unblock() can be used to block the functor's invocation
 * from operator()() temporarily.
 *
 * You should use the more convenient unnumbered sigc::slot template.
 *
 * @ingroup slot
 */
template <class T_return, class... T_arg>
class slot
  : public slot_base
{
public:
  using result_type = T_return;
  //TODO: using arg_type_ = type_trait_take_t<T_arg>;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
private:
  using rep_type = internal::slot_rep;
public:
  using call_type = T_return (*)(rep_type*, type_trait_take_t<T_arg>...);
#endif

  /** Invoke the contained functor unless slot is in blocking state.
   * @param _A_a Arguments to be passed on to the functor.
   * @return The return value of the functor invocation.
   */
  inline T_return operator()(type_trait_take_t<T_arg>... _A_a) const
    {
      if (!empty() && !blocked())
        return (reinterpret_cast<call_type>(slot_base::rep_->call_))(slot_base::rep_, _A_a...);
      return T_return();
    }

  inline slot() {}

  /** Constructs a slot from an arbitrary functor.
   * @param _A_func The desired functor the new slot should be assigned to.
   */
  template <class T_functor>
  slot(const T_functor& _A_func)
    : slot_base(new internal::typed_slot_rep<T_functor>(_A_func))
    {
      //The slot_base:: is necessary to stop the HP-UX aCC compiler from being confused. murrayc.
      slot_base::rep_->call_ = internal::slot_call<T_functor, T_return, T_arg...>::address();
    }

  /** Constructs a slot, copying an existing one.
   * @param src The existing slot to copy.
   */
  slot(const slot& src)
    : slot_base(src)
    {}

  /** Constructs a slot, moving an existing one.
   * If @p src is connected to a parent (e.g. a signal), it is copied, not moved.
   * @param src The existing slot to move or copy.
   */
  slot(slot&& src)
    : slot_base(std::move(src))
    {}

  /** Overrides this slot, making a copy from another slot.
   * @param src The slot from which to make a copy.
   * @return @p this.
   */
  slot& operator=(const slot& src)
  {
    slot_base::operator=(src);
    return *this;
  }

  /** Overrides this slot, making a move from another slot.
   * If @p src is connected to a parent (e.g. a signal), it is copied, not moved.
   * @param src The slot from which to move or copy.
   * @return @p this.
   */
  slot& operator=(slot&& src)
  {
    slot_base::operator=(std::move(src));
    return *this;
  }
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//template specialization of visitor<>::do_visit_each<>(action, functor):
/** Performs a functor on each of the targets of a functor.
 * The function overloads for sigc::slot are similar to the function
 * overloads for sigc::slot. See the description of those overloads.
 *
 * @ingroup slot
 */
template <typename T_return, typename... T_arg>
struct visitor<slot<T_return, T_arg...>>
{
  static void do_visit_each(const internal::limit_derived_target<trackable*, internal::slot_do_bind>& _A_action,
                            const slot<T_return, T_arg...>& _A_target)
  {
    if (_A_target.rep_ && _A_target.rep_->parent_ == nullptr)
    _A_target.rep_->set_parent(_A_action.action_.rep_, &internal::slot_rep::notify);
  }

  static void do_visit_each(const internal::limit_derived_target<trackable*, internal::slot_do_unbind>& _A_action,
                            const slot<T_return, T_arg...>& _A_target)
  {
    if (_A_target.rep_ && _A_target.rep_->parent_ == _A_action.action_.rep_)
      _A_target.rep_->set_parent(nullptr, nullptr);
  }

  template <typename T_action>
  static void do_visit_each(const T_action& _A_action,
                            const slot<T_return, T_arg...>& _A_target)
  {
    _A_action(_A_target);
  }
};
#endif // DOXYGEN_SHOULD_SKIP_THIS


} /* namespace sigc */


#endif /* _SIGC_FUNCTORS_SLOT_H_ */
