

#include "ASpell.hpp"

ASpell::ASpell()
{
}

ASpell::ASpell(std::string const &name, std::string const &effects) : _name(name), _effects(effects)
{
}

ASpell::ASpell(ASpell const &copy)
{
    *this = copy;
}


ASpell &ASpell::operator=(ASpell const &copy)
{
    if (this != &copy)
    {
        this->_name = copy._name;
        this->_effects = copy._effects;
    }
    return *this;
}

ASpell::~ASpell()
{
}

std::string const &ASpell::getName() const
{
    return this->_name;
}

std::string const &ASpell::getEffects() const
{
    return this->_effects;
}   

void ASpell::launch(ATarget const &target) const
{
    target.getHitBySpell(*this);
}