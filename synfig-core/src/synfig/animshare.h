#ifndef animshare_h_INCLUDED
#define animshare_h_INCLUDED

/* === H E A D E R S =================================================== */

#include "string.h"
#include "time.h"
#include "uniqueid.h"

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class AnimShare : public UniqueID {
private:
  String param_; //!< target animated glyph parameter name
  Time delay_;   //!< stagger delay
  int order_;    //!< stagger order

public:
  AnimShare();
  AnimShare(const String &param, const Time &delay, int order);

  const String &get_param() const;
  void set_param(const String &x);

  const Time &get_delay() const;
  void set_delay(const Time &x);

  int get_order() const;
  void set_order(int x);

  bool operator==(const AnimShare &rhs) const;
};

} // namespace synfig

#endif // animshare_h_INCLUDED
