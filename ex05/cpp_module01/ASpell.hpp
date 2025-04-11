

#ifndef ASPELL_HPP

#define ASPELL_HPP

#include <string>
#include <iostream>


#include "ATarget.hpp"

class ASpell
{
    private:
            std::string _name;
            std::string _effects;
            public:
                  ASpell();
                  ASpell(std::string const &name, std::string const &effects);
                  ASpell(ASpell const &copy);
                  ASpell &operator=(ASpell const &copy);
                  virtual ~ASpell();

                  std::string const &getName() const;
                  std::string const &getEffects() const;

                  void launch(ATarget const &target) const;

                  virtual ASpell *clone() const = 0;



};






#endif