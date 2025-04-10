



#ifndef WARLOCK_HPP
#define WARLOCK_HPP

# include <iostream>
# include <string>

class Warlock
{
    private :
        std::string _name;
        std::string _title;

        Warlock();
        Warlock(const Warlock &other);
        Warlock &operator=(const Warlock &other);

    public :
        Warlock(std::string const &name, std::string const &title);
        virtual ~Warlock();


        std::string const &getName() const;
        std::string const &getTitle() const;

        void setTitle(std::string const &title);
        void introduce() const;


};

#endif