#include "State.h"

namespace seuche {

int Game::totalCubes(Colour colour) const
{
    int total = 0;
    for (int city = 0; city < kCities; ++city)
        total += cubes[city][static_cast<int>(colour)];
    return total;
}

int Game::seatOf(Role role) const
{
    for (std::size_t seat = 0; seat < players.size(); ++seat) {
        if (players[seat].role == role)
            return static_cast<int>(seat);
    }
    return 0xFF;
}

bool Game::shielded(City city) const
{
    const int seat = seatOf(Role::Quarantine);
    if (seat == 0xFF || !city.valid())
        return false;
    const City guard = players[seat].city;
    return guard == city || adjacent(guard, city);
}

} // namespace seuche
