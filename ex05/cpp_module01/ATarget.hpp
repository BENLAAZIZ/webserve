

#ifndef ATarget_HPP

#define ATarget_HPP

#include <string>
#include <iostream>


class ASpell;

class ATarget
{
    private:
            std::string _type;
            public:
                  ATarget();
                  ATarget(std::string const &type);
                  virtual ~ATarget();

                  std::string const &getType() const;

                  virtual ATarget *clone() const = 0;

                  void getHitBySpell(ASpell const &spell) const;

                  

};
#include "ASpell.hpp"






#endif