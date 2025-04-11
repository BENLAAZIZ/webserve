


#include "Warlock.hpp"

Warlock::Warlock(std::string const &name, std::string const &title): _name(name), _title(title)
{
    std::cout << _name << ": This looks like another boring day." << std::endl;
} 

Warlock::~Warlock()
{
    std::cout << _name << ": My job here is done!" << std::endl;
}

std::string const &Warlock::getName() const
{
    return _name;
}

std::string const &Warlock::getTitle() const
{
    return _title;
}

void Warlock::setTitle(std::string const &title)
{
    _title = title;
}

void Warlock::introduce() const
{
    std::cout << _name << ": I am " << _name << ", " <<  _title << "!" << std::endl;
}

void Warlock::learnSpell(ASpell *spell)
{
    if (spell)
    {
        std::vector<ASpell*>::iterator ite = this->spells.end();
        for (std::vector<ASpell*>::iterator it = this->spells.begin(); it != ite; ++it)
			if ((*it)->getName() == spell->getName())
				return ;
        this->spells.push_back(spell->clone());
    }
        
}

void Warlock::forgetSpell(std::string const &spellName)
{
    std::vector<ASpell*>::iterator ite = this->spells.end();
    for (std::vector<ASpell*>::iterator it = this->spells.begin(); it != ite; ++it)
       if ((*it)->getName() == spellName)
        {
			delete *it;
			it = this->spells.erase(it);
		}
}

void Warlock::launchSpell(std::string const &spellName, ATarget const &target)
{
    std::vector<ASpell*>::iterator ite = this->spells.end();
    for (std::vector<ASpell*>::iterator it = this->spells.begin(); it != ite; ++it)
    {

        if ((*it)->getName() == spellName)
        {
            (*it)->launch(target);
            return ;
        }
    }
            
}

