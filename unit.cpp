#include<ctime>
#include<thread>

#include"include/unit.hpp"
#include"include/utils.hpp"
#include"include/level.hpp"
#include"include/colors.hpp"
#include"include/controls.hpp"
#include"include/globals.hpp"
#include"include/log.hpp"
#include<termlib/termlib.hpp>

#include<fmt/core.h>
#include<fmt/printf.h>

#include<queue>
#include<algorithm>

using namespace fmt::literals;

int g_vision = 16;
int g_maxBurden = 25;                                

extern TerminalRenderer termRend;
extern TerminalReader termRead;

Unit::Ptr unitMap[FIELD_ROWS][FIELD_COLS];

Unit::Unit(const Unit & other)
    : health(other.health)
    , maxHealth(other.maxHealth)
    , pos(other.pos)
    , symbol(other.symbol)
    , vision(other.vision)
    , weapon(), armor() {
}

Unit & Unit::operator =(const Unit & other) {
    if (this == &other) {
        return *this;
    }
    health = other.health;
    maxHealth = other.maxHealth;
    pos = other.pos;
    symbol = other.symbol;
    vision = other.vision;
    weapon = nullptr;
    armor = nullptr;
    return *this;
}

std::string Unit::getName() {
    switch (symbol) {
        case 200:
            return "Hero";
        case 201:
            return "Barbarian";
        case 202:
            return "Zombie";
    }
}

bool Unit::linearVisibilityCheck(Vec2d from, Vec2d to) const {
    Vec2d d = to - from;
    bool steep = std::abs(d.x) < std::abs(d.y);
    if (steep) {
        std::swap(d.x, d.y);
        std::swap(from.x, from.y);
    }
    double k = d.y / d.x;
    int s = sgn(d.x);
    for (int i = 0; i * s < d.x * s; i += s) {
        int x = from.x + i;
        int y = from.y + i * k;
        if (steep)
            std::swap(x, y);
        if (map[y][x] == 2)
            return false;
    }
    return true;
}

void Unit::heal(int hp) {
    health = std::min(health + hp, maxHealth);
}

bool Unit::canSee(Coord cell) const {
    double offset = 1. / VISION_PRECISION;
    Vec2d celld{ cell };
    return linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ offset, offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ offset, 1 - offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ 1 - offset, offset })
        or linearVisibilityCheck(Vec2d{ pos } + 0.5, celld + Vec2d{ 1 - offset, 1 - offset });
}

void Unit::setTo(Coord cell) {
    if (map[cell.y][cell.x] == 2 or unitMap[cell.y][cell.x] or pos == cell)
        return;

    unitMap[cell.y][cell.x] = std::move(unitMap[pos.y][pos.x]);
    pos = cell;
}

void Unit::dealDamage(int damage) {
    int defence = 0;
    if (armor != nullptr)
        defence = armor->defence;
    health -= damage * (100 - defence) / 100.f;
}

Enemy enemyTypes[Enemy::TYPES_COUNT];

Enemy::Enemy(int eType) {
    switch (eType) {
        case 0: {
            health = 7;
            maxHealth = 7;
            inventory[0] = std::make_unique<Food>(foodTypes[0]);
            inventory[1] = std::make_unique<Weapon>(weaponTypes[0]);
            weapon = dynamic_cast<Weapon *>(inventory[1].get());
            symbol = 201;
            vision = 16;
            xpCost = 3;
            break;
        }
        case 1: {
            health = 10;
            maxHealth = 10;
            inventory[0] = std::make_unique<Weapon>(weaponTypes[3]);
            weapon = dynamic_cast<Weapon *>(inventory[0].get());
            symbol = 202;
            vision = 10;
            xpCost = 2;
            break;
        }
        case 2: {
            health = 5;
            maxHealth = 5;
            inventory[0] = std::make_unique<Weapon>(weaponTypes[5]);
            inventory[1] = std::make_unique<Ammo>(ammoTypes[0]);
            weapon = dynamic_cast<Weapon *>(inventory[0].get());
            ammo = dynamic_cast<Ammo *>(inventory[1].get());
            ammo->count = std::rand() % 30 + 4;
            symbol = 203;
            vision = 16;
            xpCost = 5;
            break;
        }
    }
}

Enemy::Enemy(const Enemy & other)
    : Unit(other) {
    for (int i = 0; i < MAX_INV_SIZE; ++i) {
        if (not other.inventory[i])
            continue;
        inventory[i] = other.inventory[i]->clone();
        if (other.inventory[i].get() == other.weapon)
            weapon = dynamic_cast<Weapon *>(inventory[i].get());
        if (other.inventory[i].get() == other.armor)
            armor = dynamic_cast<Armor *>(inventory[i].get());
        if (other.inventory[i].get() == other.ammo)
            ammo = dynamic_cast<Ammo *>(inventory[i].get());
    }
}

Enemy & Enemy::operator =(const Enemy & other) {
    if (this == &other) {
        return *this;
    }
    Unit::operator =(other);
    weapon = nullptr;
    armor = nullptr;
    ammo = nullptr;
    for (int i = 0; i < MAX_INV_SIZE; ++i) {
        inventory[i].reset();
        if (not other.inventory[i])
            continue;
        inventory[i] = other.inventory[i]->clone();
        if (other.inventory[i].get() == other.weapon)
            weapon = dynamic_cast<Weapon *>(inventory[i].get());
        if (other.inventory[i].get() == other.armor)
            armor = dynamic_cast<Armor *>(inventory[i].get());
        if (other.inventory[i].get() == other.ammo)
            ammo = dynamic_cast<Ammo *>(inventory[i].get());
    }
    return *this;
}
void Enemy::dropInventory() {
    weapon = nullptr;
    armor = nullptr;
    ammo = nullptr;
    for (int i = 0; i < MAX_INV_SIZE; i++) {
        drop(std::move(inventory[i]), pos);
    }
}

char getProjectileSymbol(Direction direction) {
    switch (direction) {
        case Direction::Up:
        case Direction::Down: return '|';
        case Direction::Left:
        case Direction::Right: return '-';
        case Direction::UpRight:
        case Direction::DownLeft: return '/';
        case Direction::UpLeft:
        case Direction::DownRight: return '\\';
        default: throw std::logic_error("Unknown direction");
    }
}

void Enemy::shoot() {
    if (weapon == nullptr or ammo == nullptr)
        return;

    auto dir = directionFrom(g_hero->pos - pos).value();
    Vec2i offset = toVec2i(dir);
    char sym = getProjectileSymbol(dir);
    for (int i = 1; i < weapon->range + ammo->range; i++) {
        Coord cell = pos + offset * i;

        if (map[cell.y][cell.x] == 2)
            break;

        if (unitMap[cell.y][cell.x] and unitMap[cell.y][cell.x]->getType() == UnitHero) {
            g_hero->dealDamage(ammo->damage + weapon->damageBonus);
            break;
        }
        termRend
            .setCursorPosition(cell)
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }

    ammo->count--;
    if (ammo->count <= 0) {
        for (int i = 0; i < MAX_INV_SIZE; ++i) {
            if (inventory[i].get() == ammo) {
                inventory[i].reset();
                ammo = nullptr;
            }
        }
    }
}

std::optional<Coord> Enemy::searchForShortestPath(Coord to) const {
    if (to == pos)
        return std::nullopt;

    int maxDepth = 2 + std::abs(to.x - pos.x) + std::abs(to.y - pos.y);

    std::queue<Coord> q;
    q.push(pos);

    int used[FIELD_ROWS][FIELD_COLS] = {};
    used[pos.y][pos.x] = 1;

    std::vector<Vec2i> dirs = {
        toVec2i(Direction::Up),
        toVec2i(Direction::Down),
        toVec2i(Direction::Right),
        toVec2i(Direction::Left)
    };
    if (g_mode == 2) {
        dirs.push_back(toVec2i(Direction::UpRight));
        dirs.push_back(toVec2i(Direction::UpLeft));
        dirs.push_back(toVec2i(Direction::DownRight));
        dirs.push_back(toVec2i(Direction::DownLeft));
    }

    while (not q.empty()) {
        Coord v = q.front();
        if (v == to)
            break;

        if (used[v.y][v.x] > maxDepth)
            return std::nullopt;

        q.pop();

        for (auto dir : dirs) {
            auto tv = v + dir;
            if (tv.x >= 0 and tv.y >= 0 and tv.x < FIELD_COLS - 1 and tv.y < FIELD_ROWS - 1
                    and (not unitMap[tv.y][tv.x] or unitMap[tv.y][tv.x]->getType() == UnitHero)
                    and map[tv.y][tv.x] != 2 and used[tv.y][tv.x] == 0) {
                q.push(tv);
                used[tv.y][tv.x] = 1 + used[v.y][v.x];
            }
        }
    }

    if (not used[to.y][to.x])
        return std::nullopt;

    Coord v = to;
    while (used[v.y][v.x] > 2) {
        for (auto dir : dirs) {
            auto tv = v - dir;
            if (tv.x >= 0 and tv.y >= 0 and tv.x < FIELD_COLS - 1 and tv.y < FIELD_ROWS - 1
                    and used[tv.y][tv.x] + 1 == used[v.y][v.x]) {
                v = tv;
                break;
            }
        }
    }

    return v;
}

void Enemy::updatePosition() {
    lastTurnMoved = g_turns;

    bool canSeeHero =
        not g_hero->isInvisible()
        and distSquared(pos, g_hero->pos) < sqr(vision)
        and canSee(g_hero->pos);

    if (canSeeHero) {
        if ((pos.y == g_hero->pos.y or pos.x == g_hero->pos.x or std::abs(g_hero->pos.y - pos.y) == std::abs(g_hero->pos.x - pos.x))
                and weapon and weapon->isRanged and ammo
                and weapon->range + ammo->range >= std::abs(g_hero->pos.y - pos.y) + std::abs(g_hero->pos.x - pos.x)) {
            shoot();
        } else {
            target = g_hero->pos;

            if (auto next = searchForShortestPath(g_hero->pos)) {
                if (unitMap[next->y][next->x] and unitMap[next->y][next->x]->getType() == UnitEnemy) {
                    return;
                } else if (weapon and unitMap[next->y][next->x] and unitMap[next->y][next->x]->getType() == UnitHero) {
                    //if (weapon->getType() == ItemWeapon or unitweapon->type) {
                    if (g_hero->armor == nullptr or g_hero->armor->mdf != 2) {
                        g_hero->dealDamage(weapon->damage);
                    } else {
                        dealDamage(weapon->damage);
                    }
                        /*
                    } else if (weapon->type == ItemTools) {
                        if (g_hero->armor == nullptr or g_hero->armor->mdf != 2) {
                            g_hero->dealDamage(weapon->damage);
                        } else {
                            health -= weapon->item.invTools.damage;
                        }
                    }*/
                    if (health <= 0) {
                        unitMap[pos.y][pos.x].reset();
                        return;
                    }
                } else {
                    setTo(*next);
                    return;
                }
            } else {
                canSeeHero = false;
            }
        }
    }
    if (not canSeeHero) {
        bool needRandDir = false;
        if (target != pos) {
            if (auto next = searchForShortestPath(*target)) {
                if (next->y < FIELD_ROWS and next->y > 0 and next->x < FIELD_COLS and next->x > 0) {
                    if (weapon and unitMap[next->y][next->x] and unitMap[next->y][next->x]->getType() == UnitHero) {
                        //if (weapon->type == ItemWeapon) {
                        if (g_hero->armor == nullptr or g_hero->armor->mdf != 2) {
                            g_hero->dealDamage(weapon->damage);
                        } else {
                            dealDamage(weapon->damage);
                        }
                        /*} else if (weapon->type == ItemTools) {
                            if (g_hero->armor->item.invArmor.mdf != 2) {
                                g_hero->health -= weapon->item.invTools.damage * ((100 - g_hero->armor->item.invArmor.defence) / 100.0);
                            } else {
                                health -= weapon->item.invTools.damage;
                            }
                        }*/
                        if (health <= 0) {
                            unitMap[pos.y][pos.x].reset();
                            return;
                        }
                    } else {
                        setTo(*next);
                        return;
                    }
                }
            } else {
                needRandDir = true;
            }
        } else {
            needRandDir = true;
        }
        if (needRandDir) {
            std::vector<Coord> visibleCells;

            for (int i = std::max(pos.y - vision, 0); i < std::min(FIELD_ROWS, pos.y + vision); i++) {
                for (int j = std::max(pos.x - vision, 0); j < std::min(pos.x + vision, FIELD_COLS); j++) {
                    Vec2i cell{ j, i };
                    if (cell != pos and map[i][j] != 2
                            and distSquared(pos, cell) < sqr(vision)
                            and not unitMap[i][j] and canSee(cell)) {
                        visibleCells.push_back(cell);
                    }
                }    
            }
            int attempts = 15;
            std::optional<Coord> next;
            for (int i = 0; i < attempts; ++i) {
                target = visibleCells[std::rand() % visibleCells.size()];

                if (next = searchForShortestPath(*target)) {
                    break;
                }
            }
            if (next and next->y < FIELD_ROWS and next->y > 0 and next->x < FIELD_COLS and next->x > 0) {
                if (weapon and unitMap[next->y][next->x] and unitMap[next->y][next->x]->getType() == UnitHero) {
                    //if (weapon->type == ItemWeapon) {
                    if (g_hero->armor == nullptr or g_hero->armor->mdf != 2) {
                        g_hero->dealDamage(weapon->damage);
                    } else {
                        dealDamage(weapon->damage);
                    }
                    /*} else if (weapon->type == ItemTools) {
                        if (g_hero->armor->item.invArmor.mdf != 2) {
                            g_hero->health -= weapon->item.invTools.damage * ((100 - g_hero->armor->item.invArmor.defence) / 100.0);
                        } else {
                            health -= weapon->item.invTools.damage;
                        }
                    }*/
                    if (health <= 0) {
                        unitMap[pos.y][pos.x].reset();
                        return;
                    }
                } else {
                    setTo(*next);
                    return;
                }
            }
        }
    }
}

Hero::Hero() {
    maxHealth = 15;
    health = 15;
    symbol = 200;

    inventory[0] = std::make_unique<Armor>(armorTypes[1]);
    inventory[0]->inventorySymbol = 'a';

    armor = dynamic_cast<Armor *>(inventory[0].get());
    armor->attribute = 201;
    if (std::rand() % (500 / luck) == 0)
        armor->mdf = 2;
}

int Hero::getLevelUpXP() const{
    return level * level + 4;
}

bool Hero::tryLevelUp() {
    if (xp < getLevelUpXP())
        return false;
    levelUp();
    return true;
}

void Hero::levelUp() {
    level++;
    message += fmt::format("Now you are level {}. ", level);
    g_maxBurden += g_maxBurden / 4;
    maxHealth += maxHealth / 4;
    health = maxHealth;
}

void Hero::dealDamage(int damage) {
    int defence = 0;
    if (armor != nullptr)
        defence = armor->defence;
    health -= damage * (100 - defence) / 100.f;
}

bool Hero::isInvisible() const {
    return turnsInvisible > 0;
}

void Hero::checkVisibleCells() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            seenUpdated[i][j] = 0;
            Vec2i cell{ j, i };
            if (distSquared(pos, cell) < sqr(g_vision)) {
                seenUpdated[i][j] = canSee(cell);
            }
        }
    }
}

bool Hero::isInventoryEmpty() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i])
            return false;
    }
    return true;
}

int Hero::findEmptyInventoryCell() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (not inventory[i])
            return i;
    }
    return 101010;                                            // Magic constant, means "Inventory is full".
}

int Hero::getInventoryItemsWeight() const {
    int totalWeight = 0;
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i]) {
            totalWeight += inventory[i]->weight;
        }
    }
    return totalWeight;
}

void Hero::printList(const std::vector<Item *> & items, std::string_view msg, int mode) const {
    int num = 0;

    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, num })
        .put(msg);

    num ++;
    switch (mode) {
        case 1: {
            for (int i = 0; i < items.size(); i++) {
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
                if (items[i]->showMdf == true and items[i]->count == 1) {
                    if (items[i]->attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->getMdf()));
                    } else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->getAttribute(),
                                items[i]->getMdf()));
                    }
                } else if (items[i]->count > 1) {
                    if (items[i]->attribute == 100) {
                        termRend.put("[{}] {} {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->count));
                    } else {
                        termRend.put("[{}] {} ({}) {{{}}}. "_format(
                                items[i]->inventorySymbol,
                                items[i]->getName(),
                                items[i]->getAttribute(),
                                items[i]->count));
                    }
                } else if (items[i]->attribute == 100) {
                    termRend.put("[{}] {}. "_format(
                            items[i]->inventorySymbol,
                            items[i]->getName()));
                } else {
                    termRend.put("[{}] {} ({}). "_format(
                            items[i]->inventorySymbol,
                            items[i]->getName(),
                            items[i]->getAttribute()));
                }
                num ++;
            }
            break;
        }
        case 2: {
            for (int i = 0; i < items.size(); i++)
            {
                termRend.setCursorPosition(Vec2i{ FIELD_COLS + 10, num });
                if (items[i]->showMdf == true) {
                    termRend.put("[{}] {} ({}) {{{}}}. "_format(
                            char(i + 'a'),
                            items[i]->getName(),
                            items[i]->getAttribute(),
                            items[i]->getMdf()));
                } else {
                    termRend.put("[{}] {} ({}). "_format(
                            char(i + 'a'),
                            items[i]->getName(),
                            items[i]->getAttribute()));
                }
                num ++;
            }
            break;
        }
    }
}

bool Hero::isMapInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->symbol == 500)
            return true;
    }
    return false;
}

// 101010 something went wrong

int Hero::findAmmoInInventory() const {
    for (int i = 0; i < BANDOLIER; i++) {
        if (inventory[AMMO_SLOT + i]) {
            return i;
        }
    }
    return 101010;
}

int Hero::findScrollInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemScroll) {
            return i;
        }
    }
    return 101010;
}

void Hero::pickUpAmmo(ItemPileIter ammoIter) {                                        // Picked ammo
    clearRightPane();
    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
        .put("In what slot do you want to pull your ammo?");
    int choice = 0;
    int num = 0;
    while (true) {
        num = 0;
        for (int i = 0; i < BANDOLIER; i++) {
            termRend.setCursorPosition(Vec2i{ FIELD_COLS + num + 12, 1 });
            num += 2;
            char symbol = '-';
            TextStyle style{ TerminalColor{} };
            if (inventory[AMMO_SLOT + i]) {
                switch (inventory[AMMO_SLOT + i]->symbol) {
                    case 450:
                        symbol = ',';
                        style = TextStyle{ TextStyle::Bold, Color::Black };
                        break;
                    case 451:
                        symbol = ',';
                        style = TextStyle{ TextStyle::Bold, Color::Red };
                        break;
                    default:
                        break;
                }
            }
            if (choice == i) {
                style += TextStyle::Underlined;
            }
            termRend.put(symbol, style);
        }
        char input = termRead.readChar();
        switch (input) {
            case CONTROL_LEFT:
                if (choice > 0)
                    choice--;
                break;
            case CONTROL_RIGHT:
                if (choice < BANDOLIER - 1)
                    choice++;
                break;
            case CONTROL_CONFIRM: {
                    Item::Ptr & ammo = *ammoIter;
                    if (inventory[AMMO_SLOT + choice] == nullptr) {
                        inventory[AMMO_SLOT + choice] = std::move(ammo);
                        itemsMap[pos.y][pos.x].erase(ammoIter);
                    } else if (inventory[AMMO_SLOT + choice]->symbol == ammo->symbol) {
                        inventory[AMMO_SLOT + choice]->count += ammo->count;
                        itemsMap[pos.y][pos.x].erase(ammoIter);
                    } else {
                        std::swap(ammo, inventory[AMMO_SLOT + choice]);
                    }
                    return;
                }
            case '\033':
                return;
                break;
        }
    }
}

void Hero::pickUp() {
    if (itemsMap[pos.y][pos.x].empty()) {
        message += "There is nothing here to pick up. ";
        g_stop = true;
        return;
    } else if (itemsMap[pos.y][pos.x].size() == 1) {
        auto it = itemsMap[pos.y][pos.x].begin();
        auto & itemToPick = *it;
        message += "You picked up {}. "_format(itemToPick->getName());

        if (itemToPick->getType() == ItemAmmo) {
            pickUpAmmo(it);
            return;
        }

        bool canStack = false;

        if (itemToPick->isStackable) {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i) {
                if (inventory[i] and inventory[i]->symbol == itemToPick->symbol) {
                    canStack = true;
                    inventory[i]->count += itemToPick->count;
                    itemsMap[pos.y][pos.x].pop_back();
                }
            }
        }

        if (not canStack) {
            int eic = findEmptyInventoryCell();
            if (eic != 101010) {
                inventory[eic] = std::move(itemToPick);
                inventory[eic]->inventorySymbol = eic + 'a';
                itemsMap[pos.y][pos.x].pop_back();
            } else {
                message += "Your inventory is full, motherfuck'a! ";
            }
        }

        if (getInventoryItemsWeight() > g_maxBurden and !isBurdened) {
            message += "You're burdened. ";
            isBurdened = true;
        }

        return;
    }
    
    std::vector<Item *> list;
    for (const auto & item : itemsMap[pos.y][pos.x])
        list.push_back(item.get());

    printList(list, "What do you want to pick up? ", 2);

    int intch;
    while (true) {
        char choice = termRead.readChar();
        if (choice == '\033')
            return;

        intch = choice - 'a';
        if (intch >= 0 or intch < itemsMap[pos.y][pos.x].size())
            break;
    }

    auto itemIter = std::begin(itemsMap[pos.y][pos.x]);
    std::advance(itemIter, intch);
    auto & item = *itemIter;
    
    message += "You picked up {}. "_format(item->getName());
    
    if (item->getType() == ItemAmmo) {
        pickUpAmmo(itemIter);
        return;
    }

    bool canStack = false;

    if (item->isStackable) {
        for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i) {
            if (inventory[i] and inventory[i]->symbol == item->symbol) {
                canStack = true;
                inventory[i]->count += item->count;
                itemsMap[pos.y][pos.x].erase(itemIter);
            }
        }
    }

    if (not canStack) {
        int eic = findEmptyInventoryCell();
        if (eic != 101010) {
            inventory[eic] = std::move(item);
            inventory[eic]->inventorySymbol = eic + 'a';
            itemsMap[pos.y][pos.x].erase(itemIter);
        } else {
            message += "Your inventory is full, motherfuck'a! ";
        }
    }

    if (getInventoryItemsWeight() > g_maxBurden and !isBurdened) {
        message += "You're burdened. ";
        isBurdened = true;
    }
}

bool Hero::isFoodInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemFood)
            return true;
    }
    return false;
}

bool Hero::isArmorInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemArmor)
            return true;
    }
    return false;
}

bool Hero::isWeaponOrToolsInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and (inventory[i]->getType() == ItemWeapon or inventory[i]->getType() == ItemTools))
            return true;
    }
    return false;
}

bool Hero::isPotionInInventory() const {
    for (int i = 0; i < MAX_USABLE_INV_SIZE; i++) {
        if (inventory[i] and inventory[i]->getType() == ItemPotion)
            return true;
    }
    return false;
}

void Hero::clearRightPane() const {
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + j + 10, i })
                .put(' ');
        }
    }
}

void Hero::eat() {
    if (isFoodInInventory()) {
        showInventory(CONTROL_EAT);
    } else {
        message += "You don't have anything to eat. ";
    }
}

void Hero::processInput(char inp) {
    switch (inp) {
        case CONTROL_UP:
        case CONTROL_DOWN:
        case CONTROL_LEFT:
        case CONTROL_RIGHT:
        case CONTROL_UPLEFT:
        case CONTROL_UPRIGHT:
        case CONTROL_DOWNLEFT:
        case CONTROL_DOWNRIGHT: {
            auto offset = toVec2i(*getDirectionByControl(inp));
            moveTo(pos + offset);
            break;
        }
        case CONTROL_PICKUP: {
            pickUp();
            break;
        }
        case CONTROL_EAT: {
            eat();
            break;
        }
        case CONTROL_SHOWINVENTORY: {
            if (not isInventoryEmpty()) {
                showInventory(CONTROL_SHOWINVENTORY);
            } else {
                message += "Your inventory is empty. ";
            }
            g_stop = true;
            break;                
        }
        case CONTROL_WEAR: {
            if (isArmorInInventory()) {
                showInventory(CONTROL_WEAR);
            } else {
                message += "You don't have anything to wear. ";
            }
            g_stop = true;
            break;

        }
        case CONTROL_WIELD: {
            if (isWeaponOrToolsInInventory()) {
                showInventory(CONTROL_WIELD);
            } else {
                message += "You don't have anything to wield. ";
            }
            g_stop = true;
            break;
            }
        case CONTROL_TAKEOFF: {
            showInventory(CONTROL_TAKEOFF);
            g_stop = true;
            break;
        }
        case CONTROL_UNEQUIP: {
            showInventory(CONTROL_UNEQUIP);
            g_stop = true;
            break;
        }
        case CONTROL_DROP: {
            if (not isInventoryEmpty()) {
                showInventory(CONTROL_DROP);
            }
            g_stop = true;
            break;        
        }
        case CONTROL_THROW: {
            if (not isInventoryEmpty()) {
                showInventory(CONTROL_THROW);
            }
            break;
        }
        case CONTROL_SHOOT: {
            shoot();
            break;
        }
        case CONTROL_DRINK: {
            if (isPotionInInventory()) {
                showInventory(CONTROL_DRINK);
            }
            g_stop = true;
            break;
        }
        case CONTROL_OPENBANDOLIER: {
            if (findAmmoInInventory() != 101010) {
                showInventory(CONTROL_OPENBANDOLIER);
            } else {
                message += "Your bandolier is empty. ";
            }
            g_stop = true;
            break;
        }
        case CONTROL_RELOAD: {
            if (weapon == nullptr or not weapon->isRanged) {
                message += "You have no ranged weapon in hands. ";
                g_stop = true;
            } else if (findAmmoInInventory() != 101010) {
                showInventory(CONTROL_RELOAD);
            } else {
                message += "You have no bullets to reload. ";
                g_stop = true;
            }
            break;
        }
        case CONTROL_READ: {
            if (findScrollInInventory() != 101010) {
                showInventory(CONTROL_READ);
            } else {
                message += "You don't have anything to read. ";
            }
            g_stop = true;
            break;
        }
        case '\\': {
            char hv = termRead.readChar();
            
            if (hv == 'h') {
                if (termRead.readChar() == 'e') {
                    if (termRead.readChar() == 'a') {
                        if (termRead.readChar() == 'l') {
                            hunger = 3000;
                            health = maxHealth * 100;
                        }
                    }
                }
            }
        
            if (hv == 'w') {
                if (termRead.readChar() == 'a') {
                    if (termRead.readChar() == 'l') {
                        if (termRead.readChar() == 'l') {
                            if (termRead.readChar() == 's') {
                                canMoveThroughWalls = true;
                            }
                        }
                    }
                }
            } else if (hv == 'd') {
                if (termRead.readChar() == 's') {
                    if (termRead.readChar() == 'c') {
                        canMoveThroughWalls = false;
                    }
                } else {
                    itemsMap[1][1].push_back(std::make_unique<Food>(foodTypes[0]));
                }
            } else if (hv == 'k') {
                if (termRead.readChar() == 'i') {
                    if (termRead.readChar() == 'l') {
                        if (termRead.readChar() == 'l') {
                            health -= (health * 2) / 3;
                            message += "Ouch! ";
                        }
                    }
                }
            }
            break;
        }
    }
}

void Hero::showInventory(char inp) {    
    std::vector<Item *> list;
    auto listInserter = std::back_inserter(list);
    switch (inp) {
        case CONTROL_SHOWINVENTORY: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i])
                    list.push_back(inventory[i].get());
            
            printList(list, "Here is your inventory.", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            break;
        }
        case CONTROL_EAT: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemFood)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to eat?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            auto & item = inventory[intch];
            if (item and item->getType() == ItemFood) {
                int prob = rand() % g_hero->luck;
                if (prob == 0) {
                    hunger += dynamic_cast<Food &>(*item).nutritionalValue / 3;
                    health --;
                    message += "Fuck! This food was rotten! ";
                } else {
                    hunger += dynamic_cast<Food &>(*item).nutritionalValue;
                }
                if (item->count == 1) {
                    item.reset();
                } else {
                    item->count--;
                }
            }
            break;
        }    
        case CONTROL_WEAR: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemArmor)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to wear?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            auto & item = inventory[intch];
            if (item and item->getType() == ItemArmor) {
                message += "Now you wearing {}. "_format(item->getName());

                if (armor != nullptr) {
                    armor->attribute = 100;
                }
                armor = dynamic_cast<Armor *>(item.get());
                item->attribute = 201;
            }
            break;
        }
        case CONTROL_DROP: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i])
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to drop?", 1);
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            if (armor != nullptr and choice == armor->inventorySymbol)
                showInventory(CONTROL_TAKEOFF);
            if (weapon != nullptr and choice == weapon->inventorySymbol)
                showInventory(CONTROL_UNEQUIP);
            auto & item = inventory[intch];
            if (not item)
                break;
            if (not item->isStackable or item->count == 1) {
                drop(std::move(item), pos);
            } else {
                clearRightPane();
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                    .put("How much items do you want to drop? [1-9]");

                int dropCount = clamp(1, termRead.readChar() - '0', item->count);

                auto iter = findItemAt(pos, item->symbol);
                if (iter != end(itemsMap[pos.y][pos.x])) {        
                    (*iter)->count += dropCount;
                } else {            
                    itemsMap[pos.y][pos.x].push_back(item->clone());
                    itemsMap[pos.y][pos.x].back()->count = dropCount;
                }
                item->count -= dropCount;
                if (item->count == 0) {
                    item.reset();
                }
            }

            if (getInventoryItemsWeight() <= g_maxBurden and isBurdened) {
                message += "You are burdened no more. ";
                isBurdened = false;
            }

            break;
        }
        case CONTROL_TAKEOFF: {
            if (armor) {
                armor->attribute = 100;
                armor = nullptr;
            }
            break;
        }
        case CONTROL_WIELD: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemWeapon)
                    list.push_back(inventory[i].get());

            /*for (auto & item : list) {
                log("Offering item '{}' '{}'", item.getItem().inventorySymbol, (int) item.getItem().inventorySymbol);
            }*/

            printList(list, "What do you want to wield?", 1);
            
            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';
            auto & item = inventory[intch];
            //if (inventory[intch].type == ItemWeapon || inventory[intch].type == ItemTools) {
            if (item and item->getType() == ItemWeapon) {
                message += "You wield {}. "_format(item->getName());

                if (weapon != nullptr) {
                    weapon->attribute = 100;
                }
                weapon = dynamic_cast<Weapon *>(item.get());
                item->attribute = 301;
            }
    
            break;
        
        }
        case CONTROL_UNEQUIP: {
            if (weapon != nullptr) {
                weapon->attribute = 100;
                weapon = nullptr;
            }
            break;
        }
        case CONTROL_THROW: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i])
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to throw?", 1);

            char choice = termRead.readChar();
            if (choice == '\033') return;
            int intch = choice - 'a';

            auto & item = inventory[intch];
            if (item) {
                clearRightPane();
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
                    .put("In what direction?");
                char secondChoise = termRead.readChar();

                if (armor != nullptr and item->inventorySymbol == armor->inventorySymbol)
                    showInventory(CONTROL_TAKEOFF);
                else if (weapon != nullptr and item->inventorySymbol == weapon->inventorySymbol)
                    showInventory(CONTROL_UNEQUIP);

                throwAnimated(std::move(item), *getDirectionByControl(secondChoise));
            }
            break;
        }
        case CONTROL_DRINK: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemPotion)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to drink?", 1);

            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';

            auto & item = inventory[intch];
            if (item and item->getType() == ItemPotion) {
                auto & potion = dynamic_cast<Potion &>(*item);
                switch (potion.effect) {
                    case 1: {
                        heal(3);
                        message += "Now you feeling better. ";
                        break;
                    }
                    case 2: {
                        g_hero->turnsInvisible = 150;
                        message += "Am I invisible? Oh, lol! ";
                        break;
                    }
                    case 3: {
                        for (int i = 0; i < 1; i++) {
                            int l = rand() % FIELD_COLS;
                            int h = rand() % FIELD_ROWS;
                            if (map[h][l] != 2 and not unitMap[h][l]) {
                                setTo(Coord{ l, h });
                                checkVisibleCells();
                            } else {
                                i--;
                            }
                        }
                        message += "Teleportation is so straaange thing! ";
                        break;
                    }
                    case 4: {
                        message += "Well.. You didn't die. Nice. ";
                        break;
                    }
                    case 5: {
                        g_vision = 1;
                        g_hero->turnsBlind = 50;
                        message += "My eyes!! ";
                        break;
                    }
                }
                potionTypeKnown[item->symbol - 600] = true;
                if (item->count == 1) {
                    item.reset();
                } else {
                    --item->count;
                }
            }
            break;
        }
        case CONTROL_READ: {
            for (int i = 0; i < MAX_USABLE_INV_SIZE; ++i)
                if (inventory[i] and inventory[i]->getType() == ItemScroll)
                    list.push_back(inventory[i].get());

            printList(list, "What do you want to read?", 1);

            char choice = termRead.readChar();
            if (choice == '\033')
                return;
            int intch = choice - 'a';

            auto & item = inventory[intch];
            if (item and item->getType() == ItemScroll) {
                switch (dynamic_cast<Scroll &>(*item).effect) {
                    case 1: {
                        message += "You wrote this map. Why you read it, I don't know. ";
                        break;
                    }
                    case 2: {
                        clearRightPane();
                        termRend
                            .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                            .put("What do you want to identify?");

                        char in = termRead.readChar();
                        int intin = in - 'a';
                        auto & item2 = inventory[intin];
                        if (item2) {
                            if (item2->getType() == ItemPotion) {
                                potionTypeKnown[item2->symbol - 600] = true;
                            } else {
                                item2->showMdf = true;
                            }    
                        
                            if (item->count == 1) {
                                item.reset();
                            } else {
                                --item->count;
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }
        case CONTROL_OPENBANDOLIER: {
            clearRightPane();
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                .put("Here is your ammo.");
            int choice = 0;
            std::optional<int> takenFrom;
            while (true) {
                int num = 0;
                for (int i = 0; i < BANDOLIER; i++) {
                    num += 2;

                    TextStyle style = TextStyle{ TerminalColor{} };
                    char symbol = '-';

                    if (inventory[AMMO_SLOT + i]) {
                        switch (inventory[AMMO_SLOT + i]->symbol) {
                            case 450:
                                style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Black } };
                                symbol = ',';
                                break;
                            case 451:
                                style = TextStyle{ TextStyle::Bold, TerminalColor{ Color::Red } };
                                symbol = ',';
                                break;
                        }
                    }
                    if (choice == i)
                        style += TextStyle::Underlined;

                    termRend
                        .setCursorPosition(Vec2i{ FIELD_COLS + num + 12, 1 })
                        .put(symbol, style);
                }
                char input = termRead.readChar();
                switch (input) {
                    case CONTROL_LEFT: {
                        if (choice > 0)
                            choice--;
                        break;
                    }
                    case CONTROL_RIGHT: {
                        if (choice < BANDOLIER - 1)
                            choice++;
                        break;
                    }
                    case CONTROL_EXCHANGE: {
                        if (takenFrom) {
                            std::swap(inventory[*takenFrom], inventory[AMMO_SLOT + choice]);
                            takenFrom = std::nullopt;
                        } else if (inventory[AMMO_SLOT + choice]) {
                            takenFrom = AMMO_SLOT + choice;
                        }
                        break;
                    }
                    case '\033': {
                        return;
                        break;
                    }
                }
            }
            break;
        }
        case CONTROL_RELOAD: {
            clearRightPane();
            if (weapon == nullptr) {
                return;
            }
            termRend
                .setCursorPosition(Vec2i{ FIELD_COLS + 10 })
                .put("Now you can load your weapon");
            while (true) {
                for (int i = 0; i < weapon->maxCartridgeSize; i++) {
                    TextStyle style{ TerminalColor{} };
                    char symbol = 'i';
                    if (weapon->cartridge[i]) {
                        switch (weapon->cartridge[i]->symbol) {
                            case 450:
                                style = TextStyle{ TextStyle::Bold, Color::Black };
                                break;
                            case 451:
                                style = TextStyle{ TextStyle::Bold, Color::Red };
                                break;
                            default:
                                symbol = '?';
                        }
                    } else {
                        symbol = '_';
                    }
                    termRend
                        .setCursorPosition(Vec2i{ FIELD_COLS + i + 10, 1 })
                        .put(symbol, style);
                }
                
                std::string loadString = "";
                
                for (int i = 0; i < BANDOLIER; i++) {
                    loadString += "[{}| "_format(i + 1);
                    auto & ammo = inventory[AMMO_SLOT + i];
                    if (not ammo) {
                        loadString += "nothing ]";
                        continue;
                    }
                    switch (ammo->symbol) {
                        case 450:
                            loadString += "steel bullets";
                            break;
                        case 451:
                            loadString += "shotgun shells";
                            break;
                        default:
                            loadString += "omgwth?";
                    }
                    loadString += " ]";
                }
                
                loadString += "   [u] - unload ";
                
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 2 })
                    .put(loadString);
                
                char in = termRead.readChar();
                if (in == '\033')
                    return;

                if (in == 'u') {
                    if (weapon->currCartridgeSize == 0) {
                        continue;
                    }
                    bool found = false;
                    for (int j = 0; j < BANDOLIER; j++) {
                        auto & item = inventory[AMMO_SLOT + j];
                        if (item and item->symbol == weapon->cartridge[weapon->currCartridgeSize - 1]->symbol) {
                            weapon->cartridge[weapon->currCartridgeSize - 1].reset();
                            weapon->currCartridgeSize--;
                            item->count++;
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        continue;
                    for (int j = 0; j < BANDOLIER; j++) {
                        auto & item = inventory[AMMO_SLOT + j];
                        if (not item) {
                            item = std::move(weapon->cartridge[weapon->currCartridgeSize - 1]);
                            weapon->currCartridgeSize--;
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        continue;
                    drop(std::move(weapon->cartridge[weapon->currCartridgeSize - 1]), pos);
                    weapon->currCartridgeSize--;
                } else {
                    int intin = in - '1';
                    auto & item = inventory[AMMO_SLOT + intin];
                    if (item) {
                        if (weapon->currCartridgeSize >= weapon->maxCartridgeSize) {
                            message += "Weapon is loaded ";
                            return;
                        }
                        auto & slot = weapon->cartridge[weapon->currCartridgeSize];
                        if (item->count > 1) {
                            slot = std::make_unique<Ammo>(dynamic_cast<Ammo &>(*item));
                            slot->count = 1;
                            --item->count;
                        } else {
                            slot.reset(dynamic_cast<Ammo *>(item.release()));
                        }
                        weapon->currCartridgeSize++;
                    }
                }
            }
            break;    
        }
    }
}

void Hero::attackEnemy(Coord cell) {
    auto & enemy = dynamic_cast<Enemy &>(*unitMap[cell.y][cell.x]);
    if (weapon) {
        enemy.dealDamage(weapon->damage);
    }
    //} else if (weapon->type == ItemTools) {
        //unitMap[pos.y + a1][pos.x + a2].getUnit().health -= weapon->item.invTools.damage;
    //}
    if (enemy.health <= 0) {
        enemy.dropInventory();
        xp += enemy.xpCost;
        unitMap[cell.y][cell.x].reset();
    }
}

void Hero::throwAnimated(Item::Ptr item, Direction direction) {
    int throwDist = 0;
    auto offset = toVec2i(direction);
    char sym = getProjectileSymbol(direction);
    for (int i = 0; i < 12 - item->weight / 3; i++) {                        // 12 is "strength"
        auto[ col, row ] = pos + offset * (i + 1);
        //int row = pos.y + offset.y * (i + 1);
        //int col = pos.x + offset.x * (i + 1);

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col]) {
            unitMap[row][col]->dealDamage(item->weight / 2);
            if (unitMap[row][col]->health <= 0) {
                auto & enemy = dynamic_cast<Enemy &>(*unitMap[row][col]);
                enemy.dropInventory();
                xp += enemy.xpCost;
                unitMap[row][col].reset();
            }
            break;
        }
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
        throwDist++;
        sleep(DELAY);
    }
    drop(std::move(item), pos + offset * throwDist);
}

void Hero::shoot() {
    if (weapon == nullptr or not weapon->isRanged) {
        message += "You have no ranged weapon in hands. ";
        return;
    }
    if (weapon->currCartridgeSize == 0) {
        message += "You have no bullets. ";
        g_stop = true;
        return;
    }
    termRend
        .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
        .put("In what direction? ");

    char choice = termRead.readChar();
    auto optdir = getDirectionByControl(choice);
    if (not optdir) {
        g_stop = true;
        return;
    }
    auto direction = *optdir;
    auto offset = toVec2i(direction);
    char sym = getProjectileSymbol(direction);
    int bulletPower = weapon->cartridge[weapon->currCartridgeSize - 1]->damage + g_hero->weapon->damageBonus;

    for (int i = 1; i < weapon->range + weapon->cartridge[weapon->currCartridgeSize - 1]->range; i++) {
        auto[ col, row ] = pos + offset * i;
        //int row = pos.y + offset.y * i;
        //int col = pos.x + offset.x * i; 

        if (map[row][col] == 2)
            break;

        if (unitMap[row][col]) {
            unitMap[row][col]->dealDamage(bulletPower - i / 3);
            if (unitMap[row][col]->health <= 0) {
                auto & enemy = dynamic_cast<Enemy &>(*unitMap[row][col]);
                enemy.dropInventory();
                xp += enemy.xpCost;
                unitMap[row][col].reset();
            }
        }
        termRend
            .setCursorPosition(Vec2i{ col, row })
            .put(sym)
            .display();
        sleep(DELAY / 3);
    }
    weapon->cartridge[weapon->currCartridgeSize - 1].reset();
    weapon->currCartridgeSize--;
}

void Hero::moveTo(Coord cell) {
    if (cell.y < 0 or cell.y >= FIELD_ROWS or cell.x < 0 or cell.x >= FIELD_COLS)
        return;
    if (map[cell.y][cell.x] != 2 || canMoveThroughWalls) {
        if (unitMap[cell.y][cell.x] and unitMap[cell.y][cell.x]->getType() == UnitEnemy) {
            attackEnemy(cell);
        } else if (not unitMap[cell.y][cell.x]) {
            setTo(cell);
        }
    } else if (map[cell.y][cell.x] == 2) {
        /*if (weapon->type == ItemTools) {
            if (weapon->item.invTools.possibility == 1) {
                termRend
                    .setCursorPosition(Vec2i{ FIELD_COLS + 10, 0 })
                    .put("Do you want to dig this wall? [yn]");

                char inpChar = termRead.readChar();
                if (inpChar == 'y' || inpChar == 'Y') {
                    map[row][col] = 1;
                    weapon->item.invTools.uses--;
                    if (weapon->item.invTools.uses <= 0) {
                        message += "Your {} is broken. "_format(weapon->getItem().getName());
                        weapon->type = ItemEmpty;
                        checkVisibleCells();
                    }
                    return;
                }
            }
        }*/
        message += "The wall is the way. ";
        g_stop = true;
    }
    checkVisibleCells();
}

