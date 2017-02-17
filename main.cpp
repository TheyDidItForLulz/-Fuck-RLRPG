//////////////////////////////////////////////////////////////////////////////////////// Symbols //////////////////////////////////////////////////////////////////////////////////////////
/*
											'_'(Floor) == 1
											'#'(Wall) == 2
											'^'(Pile) == 3
											'%'(Egg) == 100
											'%'(Apple) == 101
											'@'(Hero) == 200
											'@'(Barbarian) == 201
											'@'(Zombie) == 202
											'@'(Guardian) == 203
											'&'(Leather armor) == 301
											'&'(Chain armor) == 300
											'/'(Copper shortsword) == 400
											'/'(Bronze spear) == 401
											'/'(Musket) == 402
											'/'(Stick) == 403
											'/'(Shotgun) == 404
											','(Steel bullets) == 450
											','(Shotgun shells) == 451
											'~'(Map) == 500
											'!'(Blue potion) == 600
											'\'(Pickaxe) == 700
*/
//////////////////////////////////////////////////////////////////////////////////////// Modificators /////////////////////////////////////////////////////////////////////////////////////
/*
											100 - Notning
											101..199 - Food mods
											200..299 - Armor mods
											300..399 - Weapon mods
											101 - Poisoned
											102 - Rotten
*/
//////////////////////////////////////////////////////////////////////////////////////// Attributes ///////////////////////////////////////////////////////////////////////////////////////
/*
											100 - Notning
											101..199 - Food attributes
											200..299 - Armor attributes
											300..399 - Weapon attributes
										   	201 - Worn
											301 - Wielded
*/
///////////////////////////////////////////////////////////////////////////////////////// Effects /////////////////////////////////////////////////////////////////////////////////////////
/*
 											1 - Map recording in Hard-mode								
*/
/////////////////////////////////////////////////////////////////////////////////////// Potion Effects ////////////////////////////////////////////////////////////////////////////////////
/*
 											1 - Healing 3 hp
*/
/////////////////////////////////////////////////////////////////////////////////////// Tool possibilities ////////////////////////////////////////////////////////////////////////////////
/*
   											1 - Digging through walls
*/
///////////////////////////////////////////////////////////////////////////////////// Types of weapon construction ////////////////////////////////////////////////////////////////////////
/*
 											1 - One direction(Musket), scheme:

											        @--->
											
											2 - Triple direction(Shotgun), scheme:
									
							      	 				  /
								       	 			 /
												@--->
												 \
												  \

											  !COMMENT! This isn't realized	(lol) хуй
*/
//////////////////////////////////////////////////////////////////////////////////////// Tree of skills ///////////////////////////////////////////////////////////////////////////////////
/*
                                                                                                                                                                                           
                                                                                         Detect, is food rotten                                                                            
                                                                                           /                \
                                                                                Shoot through             Chance to confuse monster                                                        
                                                                                                                                                                                           
                                                                                                                                                                                           
                                                                                                                                                                                           
                                                                                                                                                                                           
                                                                                                                                                                                           
                                                                                                                                                                                           
                                                                                                                                                                                           
                                                                                                                                                                                           
                                                                                                                                                                                           
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//!COMMENT! // Also it isn't needed to show to the player his satiation. And luck too. And enemies stuff.

#include<stdio.h>									//
#include<iostream>									//
#include<stdlib.h>									//
#include<time.h>									//
#include<ncurses.h>									//
#include<string.h>									//

#define DIR_LEFT 0
#define DIR_DOWN 1
#define DIR_UP 2
#define DIR_RIGHT 3
#define CONTROL_UP 'k'
#define CONTROL_DOWN 'j'
#define CONTROL_LEFT 'h'
#define CONTROL_RIGHT 'l'
#define CONTROL_UPLEFT 'y'
#define CONTROL_UPRIGHT 'u'
#define CONTROL_DOWNLEFT 'b'
#define CONTROL_DOWNRIGHT 'n'
#define CONTROL_PICKUP ','
#define CONTROL_EAT 'e'
#define CONTROL_SHOWINVENTORY 'i'
#define CONTROL_WEAR 'W'
#define CONTROL_WIELD 'w'
#define CONTROL_TAKEOFF 'T'
#define CONTROL_UNEQUIP '-'
#define CONTROL_DROP 'd'
#define CONTROL_THROW 't'
#define CONTROL_SHOOT 's'
#define CONTROL_DRINK 'q'
#define CONTROL_EXCHANGE 'x'
#define CONTROL_CONFIRM ' '
//#define CONTROL_RELOAD 'r'
#define CONTROL_OPENBANDOLIER 'a'
#define TypesOfFood 2									//
#define TypesOfArmor 2									//
#define TypesOfWeapon 5									//
#define TypesOfAmmo 2
#define TypesOfScroll 1
#define TypesOfPotion 1
#define TypesOfTools 1
#define TypesOfEnemies 3
#define BLACK_BLACK 1
#define	RED_BLACK 2
#define GREEN_BLACK 3
#define YELLOW_BLACK 4
#define BLUE_BLACK 5
#define MAGENTA_BLACK 6
#define CYAN_BLACK 7
#define WHITE_BLACK 8
#define BLACK_WHITE 9
#define BLACK_RED 10
#define LIGHT A_BOLD 
#define UL A_UNDERLINE
#define DELAY 0.07
#define BANDOLIER 3									// It means, that you can carry 3 types of ammo. Please, do NOT change it. It is unexpected.
#define MaxInvVol 53									//
#define TrueMaxInvVol 54+BANDOLIER							// Where 3 is supported type of ammo in inventory
#define AMMO_SLOT 53
#define EMPTY_SLOT 54
#define FOODCOUNT 10									//
#define ARMORCOUNT 4									//
#define WEAPONCOUNT 15									//
#define AMMOCOUNT 25
#define SCROLLCOUNT 0 /* JUST FOR !DEBUG!!*/
#define POTIONCOUNT 30 /* IT TOO */
#define TOOLSCOUNT 0 /* AND IT */
#define ENEMIESCOUNT 17
#define Depth MaxInvVol*2
#define VISION 16									//
#define VISION_PRECISION 256
int MaxInvItemsWeight = 25;								//
// !COMMENT! // Level-up and items stacking
// !COMMENT! // Enemies must move at first turn
int MODE = 1;	
int MenuCondition = 0;
bool EXIT = false;
bool Stop = false;
int DEFAULT_HERO_HEALTH = 10;								//

bool GenerateMap = true;
											//
using namespace std;									//

#include"include/level.hpp"
#include"include/gen_map.hpp"
#include"include/utils.hpp"
															//
int map[ FIELD_ROWS ][ FIELD_COLS ];											//
bool seenUpdated[FIELD_ROWS][FIELD_COLS];										// <- visible array
int active = 0;														//
int turns = 0; /*-1*/
															//
															//
void init_field( void )													//
{															//
	for( int i = 0; i < FIELD_ROWS; ++i )										//
	{														//
		for( int j = 0; j < FIELD_COLS; ++j )									//
		{													//
			map[ i ][ j ] = 1;										//
		}													//
	}														//
}															//
															//
void initialize( void )													//
{															//
	srand( time( 0 ) );												//
	active = 1;													//
	init_field();													//
}															//
															//
															//
string message = "";
string bar = "";
char tmp[100];

void MoveUnit(int, int&, int&, int, int);

class Item
{

public:
		
	Item(): mdf(100), showMdf(false), attribute(100), count(1){};

	int posH;
	int posL;
	int symbol;
	char inventorySymbol;
	int weight;
	int mdf;
	int attribute;
	int count;
	bool showMdf;
	bool isStackable;

	const char* GetMdf()
	{
		switch(mdf)
		{
			case 100:
				return "Nothing";
			case 101: 
				return "Poisoned";
			case 102:
				return "Rotten";
		}
	}
	const char* GetAttribute()
	{
		switch(attribute)
		{
			case 100:
				return "Nothing";
			case 201:
				return "Being worn";
			case 301:
				return "Wielded";
		}
	}
	const char* GetName()
	{
		switch(symbol)
		{
			case 100:
				return "Egg";
			case 101:
				return "Apple";
			case 300:
				return "Chain chestplate";
			case 301:
				return "Leather chestplate";
			case 400:
				return "Copper shortsword";
			case 401:
				return "Bronze spear";
			case 402:
				return "Musket";
			case 403:
				return "Stick";
			case 404:
				return "Shotgun";
			case 450:
				return "Steel bullets";
			case 451:
				return "Shotgun shells";
			case 500:
				return "Map";
			case 600:
				return "Blue potion";
			case 700:
				return "Pickaxe";
		}
	}
	~Item(){};

};

class EmptyItem: public Item
{
public:
	EmptyItem(){};
	~EmptyItem(){};
};

class Food: public Item
{
public:	
	Food(int FoodType)
	{
		switch(FoodType)
		{
			case 0:
				symbol = 100;
				FoodHeal = 100;
				weight = 1;
				break;
			case 1:
				symbol = 101;
				FoodHeal = 125;
				weight = 1;
				break;
		}
		isStackable = true;
	};
	
	int FoodHeal;
	bool isRotten;

	Food(): isRotten(false){};
	~Food(){};

};

class Armor: public Item
{
public:
	Armor(int ArmorType)
	{
		switch(ArmorType)
		{
			case 0:
				symbol = 300;
				defence = 20;
				durability = 20;
				weight = 20;
				break;
			case 1:
				symbol = 301;
				defence = 10;
				durability = 15;
				weight = 7;
				break;
		}
		isStackable = false;
	}

	int defence;
	int durability;

	Armor(){};
	~Armor(){};

};

class Weapon: public Item
{
public:
	Weapon(int WeaponType)
	{
		switch(WeaponType)
		{
			case 0:
				symbol = 400;
				damage = 2;
				weight = 3;
				Ranged = false;
				break;
			case 1:
				symbol = 401;
				damage = 3;
				weight = 5;
				Ranged = false;
				break;
			case 2:
				symbol = 402;
				damage = 1;
				weight = 3;
				range = 5;
				Ranged = true;
				damageBonus = 4;
				mSize = 1;
				current_mSize = 0;
//				type = 1;
				break;
			case 3:
				symbol = 403;
				damage = 1;
				weight = 1;
				Ranged = false;
				break;
			case 4:
				symbol = 404;
				damage = 1;
				weight = 4;
				Ranged = true;
				range = 1;
				damageBonus = 5;
				mSize = 6;
				current_mSize = 0;
//				type = 2;
				break;
		}
		isStackable = false;
	};
	
	int damage;

	int range; 									// Ranged bullets have additional effect on this paramether
	int damageBonus;								// And on this too
	bool Ranged;
//	int type;									// I think, it's nedlectful, because shotgun must shoot at three sides, but musket - only at one
	int mSize;									// Magazine size
	int current_mSize;

	Weapon(){};
	~Weapon(){};
};

class Ammo: public Item
{
public:
	Ammo(int AmmoType)
	{
		switch(AmmoType)
		{
			case 0:
				symbol = 450;
				weight = 0;
				range = 2;
				damage = 1;
				count = 1;
				break;
			case 1:
				symbol = 451;
				weight = 0;
				range = 1;
				damage = 2;
				count = 1;
				break;
		}
		isStackable = true;
	};

	int range;
	int damage;
	int count;

	Ammo(){}
	~Ammo(){}
};

class Scroll: public Item
{
public:
	Scroll(int s)
	{
		switch(s)
		{
			case 0:
				symbol = 500;
				weight = 1;
				effect = 1;
				break;
		}
		isStackable = true;
	}
	int effect;

	Scroll(){}
	~Scroll(){}
};

class Potion: public Item
{
public:
	Potion(int p)
	{
		switch(p)
		{
			case 0:
				symbol = 600;
				weight = 1;
				effect = 1;
				break;
		}
		isStackable = true;
	}
	int effect;

	Potion(){}
	~Potion(){}
};

class Tools: public Item
{
public:
	Tools(int t)
	{
		switch(t)
		{
			case 0:
				symbol = 700;
				weight = 4;
				damage = 2;
				possibility = 1;
				Ranged = false;
				range = 1;
				break;
		}
		isStackable = false;
	}
	int possibility;
	int uses;
	
	int damage;									// It is nedlectful to use weapon's attributes on tools

	int range; 									// Ranged bullets have add effect on this paramether
	bool Ranged;
	int cooldown;									// The end of using attributes

	Tools(){}
	~Tools(){}
};

enum ItemType
{
	ItemFood,
	ItemArmor,
	ItemEmpty,
	ItemWeapon,
	ItemAmmo,
	ItemScroll,
	ItemPotion,
	ItemTools
};

union InventoryItem
{
	EmptyItem invEmpty;
	Food invFood;
	Armor invArmor;
	Weapon invWeapon;
	Ammo invAmmo;
	Scroll invScroll;
	Potion invPotion;
	Tools invTools;
	InventoryItem(EmptyItem e)
	{
		invEmpty = e;
	}
	InventoryItem(Food f)
	{
		invFood = f;
	}
	InventoryItem(Armor a)
	{
		invArmor = a;
	}
	InventoryItem(Weapon w)
	{
		invWeapon = w;
	}
	InventoryItem(Ammo am)
	{
		invAmmo = am;
	}
	InventoryItem(Scroll s)
	{
		invScroll = s;
	}
	InventoryItem(Potion p)
	{
		invPotion = p;
	}
	InventoryItem(Tools t)
	{
		invTools = t;
	}
	InventoryItem(InventoryItem& i){}
	InventoryItem()
	{
		invEmpty = EmptyItem();
	}
	~InventoryItem(){}
};

struct PossibleItem
{
	InventoryItem item;
	ItemType type;
	PossibleItem(InventoryItem i, ItemType t): item(i), type(t)
	{}
	PossibleItem(){type = ItemEmpty;}
	void operator=(const Food& f)
	{
		type = ItemFood;
		item.invFood = f;
	}
	void operator=(const Armor& a)
	{
		type = ItemArmor;
		item.invArmor = a;
	}
	void operator=(const EmptyItem& e)
	{
		type = ItemEmpty;
		item.invEmpty = e;
	}
	void operator=(const Weapon& w)
	{
		type = ItemWeapon;
		item.invWeapon = w;
	}
	void operator=(const Ammo& am)
	{
		type = ItemAmmo;
		item.invAmmo = am;
	}
	void operator=(const Scroll& s)
	{
		type = ItemScroll;
		item.invScroll = s;
	}
	void operator=(const Potion& p)
	{
		type = ItemPotion;
		item.invPotion = p;
	}
	void operator=(const Tools& t)
	{
		type = ItemTools;
		item.invTools = t;
	}
	Item& GetItem()
	{
		switch(type)
		{
			case ItemFood:
				return item.invFood;
			case ItemArmor:
				return item.invArmor;
			case ItemEmpty:
				return item.invEmpty;
			case ItemWeapon:
				return item.invWeapon;
			case ItemAmmo:
				return item.invAmmo;
			case ItemScroll:
				return item.invScroll;
			case ItemPotion:
				return item.invPotion;
			case ItemTools:
				return item.invTools;
		}
	}
};

PossibleItem ItemsMap[FIELD_ROWS][FIELD_COLS][Depth];

PossibleItem inventory[TrueMaxInvVol];
int inventoryVol = 0;

Food differentFood[TypesOfFood];

Armor differentArmor[TypesOfArmor];

Weapon differentWeapon[TypesOfWeapon];

Ammo differentAmmo[TypesOfAmmo];

Scroll differentScroll[TypesOfScroll];

Potion differentPotion[TypesOfPotion];

Tools differentTools[TypesOfTools];

int Luck;

class Unit
{
public:
	Unit(): inventoryVol(0){};
	int health;
	PossibleItem unitInventory[4];
	int inventoryVol;
	PossibleItem* unitWeapon;
	PossibleItem* unitArmor;
	PossibleItem* unitAmmo;
	int posH;
	int posL;
	int symbol;
	
	const char* GetName()
	{
		switch(symbol)
		{
			case 200:
				return "Hero";
			case 201:
				return "Barbarian";
			case 202:
				return "Zombie";
		}
	}
	~Unit(){};
};

class EmptyUnit: public Unit
{
public:
	EmptyUnit(){};
	~EmptyUnit(){};
};

class Enemy: public Unit
{
public:
	Enemy(int eType): movedOnTurn( 0 ) /*-1*/
	{
		switch(eType)
		{
			case 0:
			{
				health = 10;
				unitInventory[0] = differentFood[0];
				unitInventory[1] = differentWeapon[0];
				unitWeapon = &unitInventory[1];
				inventoryVol = 2;
				symbol = 201;
				vision = 16;
				xpIncreasing = 3;
				break;
			}
			case 1:
			{
				health = 15;
				unitInventory[0] = differentWeapon[3];
				unitWeapon = &unitInventory[0];
				inventoryVol = 1;
				symbol = 202;
				vision = 10;
				xpIncreasing = 2;
				break;
			}
			case 2:
			{
				health = 7;
				unitInventory[0] = differentWeapon[2];
				unitInventory[1] = differentAmmo[0];
				unitWeapon = &unitInventory[0];
				unitAmmo = &unitInventory[1];
				unitAmmo->item.invAmmo.count = rand() % 30 + 4;
				inventoryVol = 2;
				symbol = 203;
				vision = 16;
				xpIncreasing = 5;
				break;
			}
		}
		dist = 0;
	}
	int vision;
	int dir;
	int dist;
	int movedOnTurn;
	int xpIncreasing;
		
	void Delay(double s)
	{
		clock_t clocksNow = clock();
		while(double(clock() - clocksNow) / CLOCKS_PER_SEC < s);
	}
	
	void Shoot();

	Enemy(){}
	~Enemy(){}
};

class Hero: public Unit
{
public:
	Hero(): isBurdened(false), CanHeroMoveThroughWalls(false), xp(0), level(1) {};

	int hunger;
	int xp;
	int level;
	PossibleItem* heroArmor;
	PossibleItem* heroWeapon;
	bool isBurdened;
	bool CanHeroMoveThroughWalls;
	
	bool LinearVisibilityCheck( double from_x, double from_y, double to_x, double to_y )
	{
		double dx = to_x - from_x;
		double dy = to_y - from_y;
		if( ABS( dx ) > ABS( dy ) )
		{
			double k = dy / dx;
			int s = SGN( dx );
			for( int i = 0; i * s < dx * s; i += s )
			{
				int x = from_x + i;
				int y = from_y + i * k;
				if( map[y][x] == 2 )
				{
					return false;
				}
			}
		}
		else
		{
			double k = dx / dy;
			int s = SGN( dy );
			for( int i = 0; i * s < dy * s; i += s )
			{
				int x = from_x + i * k;
				int y = from_y + i;
				if( map[y][x] == 2 )
				{
					return false;
				}
			}
		}
		return true;
	}

	bool CanSeeCell( int h, int l )
	{
		double offset = 1. / VISION_PRECISION;
		return
		LinearVisibilityCheck( posL + .5, posH + .5, l + offset, h + offset ) ||
		LinearVisibilityCheck( posL + .5, posH + .5, l + offset, h + 1 - offset ) ||
		LinearVisibilityCheck( posL + .5, posH + .5, l + 1 - offset, h + offset ) ||
		LinearVisibilityCheck( posL + .5, posH + .5, l + 1 - offset, h + 1 - offset );
	}

	void FindVisibleArray()
	{
		for(int i = 0; i < FIELD_ROWS; i++)
		{
			for(int j = 0; j < FIELD_COLS; j++)
			{
				seenUpdated[i][j] = 0;
				if( SQR( posH - i ) + SQR( posL - j ) < SQR( VISION ) )
				{
					seenUpdated[i][j] = CanSeeCell( i, j );
				}
			}
		}
	}

	void AttackEnemy(int& a1, int& a2);

	void mHLogic(int& a1, int& a2);
	
	bool isInventoryEmpty(){
	
		for(int i = 0; i < MaxInvVol; i++){

			if(inventory[i].type != ItemEmpty) return false;

		}
		return true;
	
	}

	int FindEmptyInventoryCell()
	{
		
		for(int i = 0; i < MaxInvVol; i++){

			if(inventory[i].type == ItemEmpty) return i;

		}

		return 101010;											// Magic constant, means "Inventory is full".

	}	
	
	int GetInventoryItemsWeight()
	{

		int toReturn = 0;
	
		for(int i = 0; i < MaxInvVol; i++){
			
			if(inventory[i].type != ItemEmpty){

				toReturn += inventory[i].GetItem().weight;

			}
		
		}
		
//		sprintf(tmp, "!W: %i!", toReturn);								// !DEBUG!
//		message += tmp;											//

		return toReturn;
	
	}

	void PrintList(PossibleItem items[], int len, char msg[], int mode)
	{
		int num = 0;

		move(num, Length + 10);
		printw("%s", msg);	
		num ++;
		switch(mode)
		{
			case 1:
			{
				for(int i = 0; i < len; i++)
				{
					move(num, Length + 10);
					if(items[i].GetItem().showMdf == true && items[i].GetItem().count == 1)
					{
						if( items[i].GetItem().attribute == 100 )
						{
							printw("[%c] %s {%s}. ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName(), items[i].GetItem().GetMdf());
						}
						else
						{
							printw("[%c] %s (%s) {%s}. ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName(), items[i].GetItem().GetAttribute(), items[i].GetItem().GetMdf());
						}
					}
					else if(items[i].GetItem().count > 1)
					{
						if( items[i].GetItem().attribute == 100 )
						{
							printw("[%c] %s {%i}. ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName(), items[i].GetItem().count);
						}
						else
						{
							printw("[%c] %s (%s) {%i}. ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName(), items[i].GetItem().GetAttribute(), items[i].GetItem().count);
						}
					}
					else if( items[i].GetItem().attribute == 100 )
					{
						printw("[%c] %s. ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName());
					}
					else
					{
						printw("[%c] %s (%s). ", items[i].GetItem().inventorySymbol, items[i].GetItem().GetName(), items[i].GetItem().GetAttribute());
					}
					num ++;
				}
				break;
			}
			case 2:
			{
				for(int i = 0; i < len; i++)
				{
					move(num, Length + 10);
					if(items[i].GetItem().showMdf == true)
					{
						printw("[%c] %s (%s) {%s}. ", i + 'a', items[i].GetItem().GetName(), items[i].GetItem().GetAttribute(), items[i].GetItem().GetMdf());
					}
					else printw("[%c] %s (%s). ", i + 'a', items[i].GetItem().GetName(), items[i].GetItem().GetAttribute());
					num ++;
				}
				break;

			}
		}
	}

	bool IsMapInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type != ItemEmpty && inventory[i].GetItem().symbol == 500) return true;
		}
		return false;
	}
	
	int FindElementsNumberUnderThisCell(int h, int l)
	{
		int result = 0;
		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[h][l][i].type != ItemEmpty)
			{
				result++;
			}
		}
		return result;
	}

	int FindEmptyElementUnderThisCell(int h, int l)
	{
		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[h][l][i].type == ItemEmpty)
			{
				return i;
			}
		}
		return 101010;											// Magic constant. Means, that something went wrong.
	}

	int FindNotEmptyElementUnderThisCell(int h, int l)
	{
		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[h][l][i].type != ItemEmpty)
			{
				return i;
			}
		}
		return 101010;
	}

	int FindAmmoInInventory()
	{
		for(int i = 0; i < BANDOLIER; i++)
		{
			if(inventory[AMMO_SLOT + i].type == ItemAmmo) return i;
		}
		return 101010;
	}
	
	void PrintAmmoList(PossibleItem& pAmmo)										// Picked ammo
	{
		ClearRightPane();
		move(0, Length + 10);
		printw("In what slot do vou want to pull your ammo?");
//		move(1, Length + 10);
		int choise = 0;
		int num = 0;
		while(1)
		{
			num = 0;
			for(int i = 0; i < BANDOLIER; i++)
			{
				move(1, Length + 12 + num);
				num += 2;
				if(inventory[AMMO_SLOT + i].type == ItemAmmo)
				{
					switch(inventory[AMMO_SLOT + i].GetItem().symbol)
					{
						case 450:
							if(choise == i) addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT | UL);
							else addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 451:
							if(choise == i) addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT | UL);
							else addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						default:
							if(choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
							else addch('-' | COLOR_PAIR(WHITE_BLACK));
							break;
					}
				}
				else
				{
					if(choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
					else addch('-' | COLOR_PAIR(WHITE_BLACK));
				}
			}
			switch(getch())
			{
				case CONTROL_LEFT:
					if(choise > 0) choise--;
					break;
				case CONTROL_RIGHT:
					if(choise < BANDOLIER - 1) choise++;
					break;
				case CONTROL_CONFIRM:
					if(inventory[AMMO_SLOT + choise].item.invAmmo.symbol != pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choise].type == ItemAmmo)
					{
						PossibleItem buffer;
						buffer = pAmmo;
						pAmmo = inventory[AMMO_SLOT + choise];
						inventory[AMMO_SLOT + choise] = buffer;
					}
					else if(inventory[AMMO_SLOT + choise].item.invAmmo.symbol == pAmmo.item.invAmmo.symbol && inventory[AMMO_SLOT + choise].type == ItemAmmo)
					{
						inventory[AMMO_SLOT + choise].item.invAmmo.count += pAmmo.item.invAmmo.count;
						pAmmo.type = ItemEmpty;
					}
					else if(inventory[AMMO_SLOT + choise].type == ItemEmpty)
					{
						inventory[AMMO_SLOT + choise] = pAmmo;
						pAmmo.type = ItemEmpty;
					}
					return;
					break;
				case '\033':
					return;
					break;
			}
		}
	}

	void PickUp(){
		
		if(FindElementsNumberUnderThisCell(posH, posL) == 0)
		{
			message += "There is nothing here to pick up. ";
			Stop = true;
			return;
		}
		else if(FindElementsNumberUnderThisCell(posH, posL) == 1)
		{
			int num = FindNotEmptyElementUnderThisCell(posH, posL);

			sprintf(tmp, "You picked up %s. ", ItemsMap[posH][posL][num].GetItem().GetName());
			message += tmp;

			if(ItemsMap[posH][posL][num].type == ItemAmmo)
			{
				PrintAmmoList(ItemsMap[posH][posL][num]);
				return;
			}

			bool couldStack = false;

			if( ItemsMap[posH][posL][num].GetItem().isStackable )
			{
				for( int i = 0; i < MaxInvVol; ++i )
				{
					if( inventory[i].type != ItemEmpty && inventory[i].GetItem().symbol == ItemsMap[posH][posL][num].GetItem().symbol )
					{
						couldStack = true;
						inventory[i].GetItem().count += ItemsMap[posH][posL][num].GetItem().count;
						ItemsMap[posH][posL][num].type = ItemEmpty;
					}
				}
			}

			if( !couldStack )
			{
				int eic = FindEmptyInventoryCell();
				if(eic != 101010)
				{
					inventory[eic] = ItemsMap[posH][posL][num];
					inventory[eic].GetItem().inventorySymbol = eic + 'a';
					ItemsMap[posH][posL][num].type = ItemEmpty;
					inventoryVol++;
				}
				else
				{
					message += "Your inventory is full, motherfuck'a! ";
				}
			}

			if(GetInventoryItemsWeight() > MaxInvItemsWeight && !isBurdened)
			{
				message += "You're burdened. ";
				isBurdened = true;
			}

			return;
		}
		
		PossibleItem list[Depth];
		char hv[200] = "What do you want to pick up? ";
		int len = 0;

		for(int i = 0; i < Depth; i++)
		{	
			if(ItemsMap[posH][posL][i].type != ItemEmpty)
			{
				list[len] = ItemsMap[posH][posL][i];
				len++;
			}
		}

		PrintList(list, len, hv, 2);
		len = 0;

		char choise = getch();

		if(choise == '\033') return;

		int intch = choise - 'a';
		
		int helpfulArray[Depth], hACounter = 0;

		for(int i = 0; i < Depth; i++)
		{
			if(ItemsMap[posH][posL][i].type != ItemEmpty)
			{
				helpfulArray[hACounter] = i;
				hACounter++;
			}
		}

		if(ItemsMap[posH][posL][helpfulArray[intch]].type != ItemEmpty)
		{
			sprintf(tmp, "You picked up %s. ", ItemsMap[posH][posL][helpfulArray[intch]].GetItem().GetName());
			message += tmp;
			
			if(ItemsMap[posH][posL][helpfulArray[intch]].type == ItemAmmo)
			{
				PrintAmmoList(ItemsMap[posH][posL][helpfulArray[intch]]);
				return;
			}

			bool couldStack = false;

			if( ItemsMap[posH][posL][helpfulArray[intch]].GetItem().isStackable )
			{
				for( int i = 0; i < MaxInvVol; ++i )
				{
					if( inventory[i].type != ItemEmpty && inventory[i].GetItem().symbol == ItemsMap[posH][posL][helpfulArray[intch]].GetItem().symbol )
					{
						couldStack = true;
						inventory[i].GetItem().count += ItemsMap[posH][posL][helpfulArray[intch]].GetItem().count;
						ItemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
					}
				}
			}

			if( !couldStack )
			{
				int eic = FindEmptyInventoryCell();
				if(eic != 101010)
				{
					inventory[eic] = ItemsMap[posH][posL][helpfulArray[intch]];
					inventory[eic].GetItem().inventorySymbol = eic + 'a';
					ItemsMap[posH][posL][helpfulArray[intch]].type = ItemEmpty;
					inventoryVol++;
				}
				else
				{
					message += "Your inventory is full, motherfuck'a! ";
				}
			}
		}
	
		if(GetInventoryItemsWeight() > MaxInvItemsWeight && !isBurdened)
		{
			message += "You're burdened. ";
			isBurdened = true;
		}
	}

	bool isFoodInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type == ItemFood) return true;
		}
		return false;
	}
	
	bool isArmorInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type == ItemArmor) return true;
		}
		return false;
	}

	bool isWeaponOrToolsInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type == ItemWeapon || inventory[i].type == ItemTools) return true;
		}
		return false;
	}

	bool isPotionInInventory()
	{
		for(int i = 0; i < MaxInvVol; i++)
		{
			if(inventory[i].type == ItemPotion) return true;
		}
		return false;
	}
	
	void ClearRightPane()
	{
		for(int i = 0; i < 100; i++)
		{
			for(int j = 0; j < 50; j++)
			{
				move(i, Length + 10 + j);
				addch(' ');
			}
		}
	}

	void Delay(double s)
	{
		clock_t clocksNow = clock();
		while(double(clock() - clocksNow) / CLOCKS_PER_SEC < s);
	}
	
	void ThrowAnimated(PossibleItem& item, char direction);

	void Shoot();
	
	void ShowInventory(const char& inp){
		
		PossibleItem list[MaxInvVol];

		int len = 0;

		switch(inp){
			
			case CONTROL_SHOWINVENTORY:
			{
				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type != ItemEmpty)
					{
						list[len] = inventory[i];
						len++;
					}
				}
				
				char hv[200] = "Here is your inventory.";

				PrintList(list, len, hv, 1);
					
				char choise = getch();
		
				if(choise == '\033') return;

				len = 0;

				break;
			}

			case CONTROL_EAT:
			{
				
				char hv[200] = "What do you want to eat?";

				for(int i = 0; i < MaxInvVol; i++){
				
					if(inventory[i].type == ItemFood)
					{
						list[len] = inventory[i];
						len++;
					}
		
				}
				
				PrintList(list, len, hv, 1);

				len = 0;

				char choise = getch();

				if(choise == '\033') return;
				
				int intch = choise - 'a';

				if(inventory[intch].type == ItemFood)
				{
					int prob = rand() % Luck;
					if(prob == 0)
					{
						hunger += inventory[intch].item.invFood.FoodHeal / 3;
						health --;
						message += "Fuck! This food was rotten! ";
					}
					else
					{
						hunger += inventory[intch].item.invFood.FoodHeal;
					}
					if(inventory[intch].GetItem().count == 1)
					{
						inventory[intch].type = ItemEmpty;
					}
					else
					{
						inventory[intch].GetItem().count--;
					}
				}
				
				break;
			}	
			case CONTROL_WEAR:
			{
				
				char hv[200] = "What do you want to wear?";
				
				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type == ItemArmor)
					{
						list[len] = inventory[i];
						len++;
					}
				}
				PrintList(list, len, hv, 1);
				len = 0;

				char choise = getch();

				if(choise == '\033') return;
	
				int intch = choise - 'a';

				if(inventory[intch].type == ItemArmor)
				{
					sprintf(tmp, "Now you wearing %s. ", inventory[intch].GetItem().GetName());
					message += tmp;

					if(heroArmor->type != ItemEmpty)
					{
						heroArmor->GetItem().attribute = 100;
					}
					heroArmor = &inventory[intch];
					inventory[intch].GetItem().attribute = 201;
				}
	
				break;

			}

			case CONTROL_DROP:
			{
				char hv[200] = "What do you want to drop?";

				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type != ItemEmpty)
					{
						list[len] = inventory[i];
						len++;
					}
				}

				PrintList(list, len, hv, 1);
				len = 0;

				char choise = getch();

				if(choise == '\033') return;
				
				int intch = choise - 'a';
				
				int num = FindEmptyElementUnderThisCell(posH, posL);
				if(num == 101010)
				{
					message += "There is too much items";
					return;
				}
				
				if(choise == heroArmor->GetItem().inventorySymbol) ShowInventory(CONTROL_TAKEOFF);
				if(choise == heroWeapon->GetItem().inventorySymbol) ShowInventory(CONTROL_UNEQUIP);

				ItemsMap[posH][posL][num] = inventory[intch];
				inventory[intch].type = ItemEmpty;

				if(GetInventoryItemsWeight() <= MaxInvItemsWeight && isBurdened)
				{
					message += "You are burdened no more. ";
					isBurdened = false;
				}

				break;
			}
			case CONTROL_TAKEOFF:
			{
				
				heroArmor->GetItem().attribute = 100;
				heroArmor = &inventory[EMPTY_SLOT];
				break;
			
			}
			case CONTROL_WIELD:
			{
				char hv[200] = "What do you want to wield?";

				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type == ItemWeapon || inventory[i].type == ItemTools)
					{
						list[len] = inventory[i];
						len++;
					}
				}

				PrintList(list, len, hv, 1);
				len = 0;
				
				char choise = getch();

				if(choise == '\033') return;
				
				int intch = choise - 'a';
		
				if(inventory[intch].type == ItemWeapon || inventory[intch].type == ItemTools)
				{
					sprintf(tmp, "You wield %s.", inventory[intch].GetItem().GetName());
					message += tmp;

					if(heroWeapon->type != ItemEmpty)
					{
						heroWeapon->GetItem().attribute = 100;
					}
					heroWeapon = &inventory[intch];
					inventory[intch].GetItem().attribute = 301;
				}
		
				break;
			
			}
			case CONTROL_UNEQUIP:
			{
				heroWeapon->GetItem().attribute = 100;
				heroWeapon = &inventory[EMPTY_SLOT];
				break;
			}
			case CONTROL_THROW:
			{
				char hv[200] = "What do you want to throw?";

				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type != ItemEmpty)
					{
						list[len] = inventory[i];
						len++;
					}
				}

				PrintList(list, len, hv, 1);
				len = 0;

				char choise = getch();
				if(choise == '\033') return;
				int intch = choise - 'a';

				if(inventory[intch].type != ItemEmpty)
				{
					ClearRightPane();
					move(0, Length + 10);
					printw("In what direction?");
					char secondChoise = getch();
					if(inventory[intch].GetItem().inventorySymbol == heroArmor->GetItem().inventorySymbol) ShowInventory(CONTROL_TAKEOFF);
					if(inventory[intch].GetItem().inventorySymbol == heroWeapon->GetItem().inventorySymbol) ShowInventory(CONTROL_UNEQUIP);
					ThrowAnimated(inventory[intch], secondChoise);
				}
				break;
			}
			case CONTROL_DRINK:
			{
				char hv[200] = "What do you want to drink?";

				for(int i = 0; i < MaxInvVol; i++)
				{
					if(inventory[i].type == ItemPotion)
					{
						list[len] = inventory[i];
						len++;
					}
				}

				PrintList(list, len, hv, 1);
				len = 0;

				char choise = getch();
				if(choise == '\033') return;
				int intch = choise - 'a';

				if(inventory[intch].type == ItemPotion)
				{
					switch(inventory[intch].item.invPotion.effect)
					{
						case 1:
							health += 3;
							if(health > DEFAULT_HERO_HEALTH)
							{
								health = DEFAULT_HERO_HEALTH;
							}
							message += "Now you feeling better. ";
							break;
					}
					if( inventory[intch].GetItem().count == 1 )
					{
						inventory[intch].type = ItemEmpty;
					}
					else
					{
						--inventory[intch].GetItem().count;
					}
				}
				break;
			}
			case CONTROL_OPENBANDOLIER:
			{
				ClearRightPane();
				move(0, Length + 10);
				printw("Here is your ammo.");
		//		move(1, Length + 10);
				int choise = 0;
				int num = 0;
				PossibleItem buffer;
				int pos;
				while(1)
				{
					num = 0;
					for(int i = 0; i < BANDOLIER; i++)
					{
						move(1, Length + 12 + num);
						num += 2;
						if(inventory[AMMO_SLOT + i].type == ItemAmmo)
						{
							switch(inventory[AMMO_SLOT + i].GetItem().symbol)
							{
								case 450:
									if(choise == i) addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT | UL);
									else addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
									break;
								case 451:
									if(choise == i) addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT | UL);
									else addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
									break;
								default:
									if(choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
									else addch('-' | COLOR_PAIR(WHITE_BLACK));
									break;
							}
						}
						else
						{
							if(choise == i) addch('-' | COLOR_PAIR(WHITE_BLACK) | UL);
							else addch('-' | COLOR_PAIR(WHITE_BLACK));
						}
					}
					switch(getch())
					{
						case CONTROL_LEFT:
						{
							if(choise > 0) choise--;
							break;
						}
						case CONTROL_RIGHT:
						{
							if(choise < BANDOLIER - 1) choise++;
							break;
						}
						case CONTROL_EXCHANGE:
						{
							if(buffer.type != ItemEmpty)
							{
								inventory[pos] = inventory[AMMO_SLOT + choise];
								inventory[AMMO_SLOT + choise] = buffer;
								buffer.type = ItemEmpty;
							}
							else
							{
								buffer = inventory[AMMO_SLOT + choise];
								inventory[AMMO_SLOT + choise].type = ItemEmpty;
								pos = AMMO_SLOT + choise;
							}
							break;
						}
						case '\033':
						{
							if(buffer.type != ItemEmpty)
							{
								inventory[pos].type = ItemAmmo;
								buffer.type = ItemEmpty;
							}
							return;
							break;
						}
					}
				}
				break;
			}
		}
	}
	
	void Eat()
	{
		if(isFoodInInventory())
		{
			ShowInventory(CONTROL_EAT);
		}
		else message += "You don't have anything to eat. ";
	}

	void moveHero(char& inp){
		
		int a1 = 0, a2 = 0;
		
		switch(inp){
			
			case CONTROL_UP:
			{
				a1 --;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_DOWN:
			{
				a1 ++;
				mHLogic(a1, a2);
				break;	
			}
			case CONTROL_LEFT:
			{
				a2 --;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_RIGHT:
			{
				a2 ++;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_UPLEFT:
			{
				a1 --;
				a2 --;
				mHLogic(a1, a2);
				break;	
			}
			case CONTROL_UPRIGHT:
			{
				a2 ++;
				a1 --;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_DOWNLEFT:
			{
				a1 ++;
				a2 --;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_DOWNRIGHT:
			{
				a1 ++;
				a2 ++;
				mHLogic(a1, a2);
				break;
			}
			case CONTROL_PICKUP:
			{
				
				PickUp();
				break;

			}
			case CONTROL_EAT:
			{

				Eat();
				break;

			}
			case CONTROL_SHOWINVENTORY:
			{
				if(isInventoryEmpty() == false)
				{
					ShowInventory(CONTROL_SHOWINVENTORY);
				}
				else
				{
					message += "Your inventory is empty. ";
				}
				break;				
			}
			case CONTROL_WEAR:
			{

				if(isArmorInInventory() == true){
				
					ShowInventory(CONTROL_WEAR);

				}
				else message += "You don't have anything to wear. ";
				break;

			}
			case CONTROL_WIELD:
			{
			
				if(isWeaponOrToolsInInventory() == true)
				{
					ShowInventory(CONTROL_WIELD);
				}
				else message += "You don't have anything to wield. ";
				break;
			
			}
			case CONTROL_TAKEOFF:
			{
				ShowInventory(CONTROL_TAKEOFF);
				break;
			}
			case CONTROL_UNEQUIP:
			{
				ShowInventory(CONTROL_UNEQUIP);
				break;
			}
			case CONTROL_DROP:
			{
				if(isInventoryEmpty() == false)
				{
					ShowInventory(CONTROL_DROP);
				}
				break;		
			}
			case CONTROL_THROW:
			{
				if(isInventoryEmpty() == false)
				{
					ShowInventory(CONTROL_THROW);
				}
				break;
			}
			case CONTROL_SHOOT:
			{
				Shoot();
				break;
			}
			case CONTROL_DRINK:
			{
				if(isPotionInInventory() == true)
				{
					ShowInventory(CONTROL_DRINK);
				}
				break;
			}
//			case CONTROL_RELOAD:
//			{
//				if(heroWeapon->item.invWeapon.Ranged && heroWeapon->item.invWeapon.current_mSize < heroWeapon->item.invWeapon.mSize)
//				{
//					Reload
//				}
//				break;
//			}
			case CONTROL_OPENBANDOLIER:
			{
				if(FindAmmoInInventory() != 101010)
				{
					ShowInventory(CONTROL_OPENBANDOLIER);
				}
				else message += "Your bandolier is empty";
				break;
			}
			case '\\':
			{
				char hv = getch();
				
				if(hv == 'h')
				{
					if(getch() == 'e')
					{
						if(getch() == 'a')
						{
							if(getch() == 'l')
							{
								hunger = 3000;
								health = DEFAULT_HERO_HEALTH * 100;
							}
						}
					}
				}
			
				if(hv == 'w')
				{
					if(getch() == 'a')
					{
						if(getch() == 'l')
						{
							if(getch() == 'l')
							{
								if(getch() == 's')
								{
									CanHeroMoveThroughWalls = true;
								}
							}
						}
					}
				}
				else if(hv == 'd')
				{
					if(getch() == 's')
					{
						if(getch() == 'c')
						{
							CanHeroMoveThroughWalls = false;
						}
					}
					else
					{
						ItemsMap[1][1][0] = differentFood[0];
					}

				}
				else if(hv == 'k')
				{
					if(getch() == 'i')
					{
						if(getch() == 'l')
						{
							if(getch() == 'l')
							{
								health -= (DEFAULT_HERO_HEALTH * 2) / 3;
								message += "Ouch! ";
							}
						}
					}
				}
				break;
			}
		}
	}
};

Hero hero;

enum UnitType
{
	UnitEmpty,
	UnitHero,
	UnitEnemy
};

union UnitedUnits
{
	EmptyUnit uEmpty;
	Hero uHero;
	Enemy uEnemy;
	UnitedUnits(EmptyUnit e)
	{
		uEmpty = e;
	}
	UnitedUnits(Hero h)
	{
		uHero = h;
	}
	UnitedUnits(Enemy en)
	{
		uEnemy = en;
	}
	UnitedUnits(UnitedUnits& u){}
	UnitedUnits()
	{
		uEmpty = EmptyUnit();
	}
	~UnitedUnits(){}
};

struct PossibleUnit
{
	UnitedUnits unit;
	UnitType type;
	PossibleUnit(UnitedUnits u, UnitType t): unit(u), type(t) {}
	PossibleUnit()
	{
		type = UnitEmpty;
	}
	void operator=(const Hero& h)
	{
		type = UnitHero;
		unit.uHero = h;
	}
	void operator=(const EmptyUnit& e)
	{
		type = UnitEmpty;
		unit.uEmpty = e;
	}
	void operator=(const Enemy& en)
	{
		type = UnitEnemy;
		unit.uEnemy = en;
	}
	Unit& GetUnit()
	{
		switch(type)
		{
			case UnitEmpty:
				return unit.uEmpty;
			case UnitHero:
				return unit.uHero;
			case UnitEnemy:
				return unit.uEnemy;
		}		
	}
};

PossibleUnit UnitsMap[FIELD_ROWS][FIELD_COLS];

Enemy differentEnemies[TypesOfEnemies];

void DropInventory(PossibleUnit& unit)
{
	int h = unit.GetUnit().posH;
	int l =	unit.GetUnit().posL;
	for(int i = 0; i < 4; i++)
	{
		if(unit.GetUnit().unitInventory[i].type != ItemEmpty)
		{
			int empty = hero.FindEmptyElementUnderThisCell(h, l);
			if(empty != 101010)
			{
				ItemsMap[h][l][empty] = unit.GetUnit().unitInventory[i];
			}
		}
	}
//	sprintf(tmp, "!%i!", unit.unit.uEnemy.unitInventory[0].item.invWeapon/*differentFood[0]*/.damage);
//	message += tmp;
}

void Hero::AttackEnemy(int& a1, int& a2)
{
	if(heroWeapon->type == ItemWeapon)
	{
		UnitsMap[posH + a1][posL + a2].GetUnit().health -= heroWeapon->item.invWeapon.damage;
	}
	else if(heroWeapon->type == ItemTools)
	{
		UnitsMap[posH + a1][posL + a2].GetUnit().health -= heroWeapon->item.invTools.damage;
	}
	if(UnitsMap[posH + a1][posL + a2].GetUnit().health <= 0)
	{
		DropInventory(UnitsMap[posH + a1][posL + a2]);
//		sprintf(tmp, "!%i!", UnitsMap[posH + a1][posL + a2].GetUnit().unitInventory[0].GetItem().symbol);
//		message += tmp;
		UnitsMap[posH + a1][posL + a2].type = UnitEmpty;
		xp += UnitsMap[posH + a1][posL + a2].unit.uEnemy.xpIncreasing;
	}
}

void Hero::ThrowAnimated(PossibleItem& item, char direction)
{
	int ThrowLength = 0;

	switch(direction)
	{
		case CONTROL_RIGHT:
		{
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)							// 12 is "strength"
			{
				if(map[posH][posL + i + 1] == 2) break;
				if(UnitsMap[posH][posL + i + 1].type != UnitEmpty)
				{
					UnitsMap[posH][posL + i + 1].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH][posL + i + 1].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL + i + 1]);
						UnitsMap[posH][posL + i + 1].type = UnitEmpty;
						xp += UnitsMap[posH][posL + i + 1].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH, posL + i + 1);
				addch('-');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyElementUnderThisCell(posH, posL + ThrowLength);
			if(empty == 101010)
			{
				int empty2 = FindEmptyElementUnderThisCell(posH, posL + ThrowLength - 1);
				ItemsMap[posH][posL + ThrowLength - 1][empty2] = item;
				item.type = ItemEmpty;
			}
			else
			{
				ItemsMap[posH][posL + ThrowLength][empty] = item;
				item.type = ItemEmpty;
			}
			break;
		}
		case CONTROL_LEFT:
		{
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)
			{
				if(map[posH][posL - i - 1] == 2) break;
				if(UnitsMap[posH][posL - i - 1].type != UnitEmpty)
				{
					UnitsMap[posH][posL - i - 1].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH][posL - i - 1].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL - i - 1]);
						UnitsMap[posH][posL - i - 1].type = UnitEmpty;
						xp += UnitsMap[posH][posL - i - 1].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH, posL - i - 1);
				addch('-');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyElementUnderThisCell(posH, posL - ThrowLength);
			if(empty == 101010)
			{
				int empty2 = FindEmptyElementUnderThisCell(posH, posL - ThrowLength + 1);
				ItemsMap[posH][posL - ThrowLength + 1][empty2] = item;
				item.type = ItemEmpty;
			}
			else
			{
				ItemsMap[posH][posL - ThrowLength][empty] = item;
				item.type = ItemEmpty;
			}
			break;
		}
		case CONTROL_UP:
		{
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)
			{
				if(map[posH - i - 1][posL] == 2) break;
				if(UnitsMap[posH - i - 1][posL].type != UnitEmpty)
				{
					UnitsMap[posH - i - 1][posL].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH - i - 1][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i - 1][posL]);
						UnitsMap[posH - i - 1][posL].type = UnitEmpty;
						xp += UnitsMap[posH - i - 1][posL].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH - i - 1, posL);
				addch('|');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyElementUnderThisCell(posH - ThrowLength, posL);
			if(empty == 101010)
			{
				int empty2 = FindEmptyElementUnderThisCell(posH - ThrowLength + 1, posL);
				ItemsMap[posH - ThrowLength + 1][posL][empty2] = item;
				item.type = ItemEmpty;
			}
			else
			{
				ItemsMap[posH - ThrowLength][posL][empty] = item;
				item.type = ItemEmpty;
			}
			break;
		}
		case CONTROL_DOWN:
		{
			for(int i = 0; i < 12 - item.GetItem().weight / 3; i++)
			{
				if(map[posH + i + 1][posL] == 2) break;
				if(UnitsMap[posH + i + 1][posL].type != UnitEmpty)
				{
					UnitsMap[posH + i + 1][posL].GetUnit().health -= item.GetItem().weight / 2;
					if(UnitsMap[posH + i + 1][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i + 1][posL]);
						UnitsMap[posH + i + 1][posL].type = UnitEmpty;
						xp += UnitsMap[posH + 1 + i][posL].unit.uEnemy.xpIncreasing;
					}
					break;
				}
				move(posH + i + 1, posL);
				addch('|');
				refresh();
				ThrowLength++;
				Delay(DELAY);
			}
			int empty = FindEmptyElementUnderThisCell(posH + ThrowLength, posL);
			if(empty == 101010)
			{
				int empty2 = FindEmptyElementUnderThisCell(posH + ThrowLength - 1, posL);
				ItemsMap[posH + ThrowLength - 1][posL][empty2] = item;
				item.type = ItemEmpty;
			}
			else
			{
				ItemsMap[posH + ThrowLength][posL][empty] = item;
				item.type = ItemEmpty;
			}
			break;
		}
	}
}

void Hero::Shoot()
{
	if(heroWeapon->item.invWeapon.Ranged == false)
	{
		message += "You have no ranged weapon in hands. ";
		return;
	}
	int ammo_slot = FindAmmoInInventory();
	if(ammo_slot == 101010)
	{
		message += "You have no bullets. ";
		return;
	}
	else if(inventory[ammo_slot + AMMO_SLOT].item.invAmmo.count == 0)
	{
		message += "You have no bullets. ";
		return;
	}
	ammo_slot += AMMO_SLOT;
	ClearRightPane();
	move(Length + 10, 0);
	printw("In what direction? ");
	char choise = getch();
	switch(choise)
	{
		case CONTROL_LEFT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH][posL - i] == 2) break;
				if(UnitsMap[posH][posL - i].type != UnitEmpty)
				{
					UnitsMap[posH][posL - i].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;
					if(UnitsMap[posH][posL - i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL - i]);
						UnitsMap[posH][posL - i].type = UnitEmpty;
						xp += UnitsMap[posH][posL - i].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH][posL - i].GetUnit().health);
//					message += tmp;
				}
// !COMMENT!			// You can make this (^) bullet moving through like a special skill
				move(posH, posL - i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_DOWN:
		{	
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH + i][posL] == 2) break;
				if(UnitsMap[posH + i][posL].type != UnitEmpty)
				{
					UnitsMap[posH + i][posL].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;	
					if(UnitsMap[posH + i][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i][posL]);
						UnitsMap[posH + i][posL].type = UnitEmpty;
						xp += UnitsMap[posH + i][posL].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH + i][posL].GetUnit().health);
//					message += tmp;
				}
				move(posH + i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_UP:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH - i][posL] == 2) break;
				if(UnitsMap[posH - i][posL].type != UnitEmpty)
				{
					UnitsMap[posH - i][posL].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;		
					if(UnitsMap[posH - i][posL].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i][posL]);
						UnitsMap[posH - i][posL].type = UnitEmpty;
						xp += UnitsMap[posH - i][posL].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH - i][posL].GetUnit().health);
//					message += tmp;
				}
				move(posH - i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_RIGHT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH][posL + i] == 2) break;
				if(UnitsMap[posH][posL + i].type != UnitEmpty)
				{
					UnitsMap[posH][posL + i].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;	
					if(UnitsMap[posH][posL + i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH][posL + i]);
						UnitsMap[posH][posL + i].type = UnitEmpty;
						xp += UnitsMap[posH][posL + i].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH][posL + i].GetUnit().health);
//					message += tmp;
				}
				move(posH, posL + i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_UPLEFT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH - i][posL - i] == 2) break;
				if(UnitsMap[posH - i][posL - i].type != UnitEmpty)
				{
					UnitsMap[posH - i][posL - i].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;	
					if(UnitsMap[posH - i][posL - i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i][posL - i]);
						UnitsMap[posH - i][posL - i].type = UnitEmpty;
						xp += UnitsMap[posH - i][posL - i].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH - i][posL - i].GetUnit().health);
//					message += tmp;
				}
				move(posH - i, posL - i);
				addch('\\');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_UPRIGHT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH - i][posL + i] == 2) break;
				if(UnitsMap[posH - i][posL + i].type != UnitEmpty)
				{
					UnitsMap[posH - i][posL + i].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;	
					if(UnitsMap[posH - i][posL + i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH - i][posL + i]);
						UnitsMap[posH - i][posL + i].type = UnitEmpty;
						xp += UnitsMap[posH - i][posL + i].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH - i][posL + i].GetUnit().health);
//					message += tmp;
				}
				move(posH - i, posL + i);
				addch('/');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_DOWNLEFT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH + i][posL - i] == 2) break;
				if(UnitsMap[posH + i][posL - i].type != UnitEmpty)
				{
					UnitsMap[posH + i][posL - i].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;	
					if(UnitsMap[posH + i][posL - i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i][posL - i]);
						UnitsMap[posH + i][posL - i].type = UnitEmpty;
						xp += UnitsMap[posH + i][posL - i].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH + i][posL - i].GetUnit().health);
//					message += tmp;
				}
				move(posH + i, posL - i);
				addch('/');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case CONTROL_DOWNRIGHT:
		{
			for(int i = 1; i < heroWeapon->item.invWeapon.range + inventory[ammo_slot].item.invAmmo.range; i++)
			{
				if(map[posH + i][posL + i] == 2) break;
				if(UnitsMap[posH + i][posL + i].type != UnitEmpty)
				{
					UnitsMap[posH + i][posL + i].GetUnit().health -= inventory[ammo_slot].item.invAmmo.damage + hero.heroWeapon->item.invWeapon.damageBonus;	
					if(UnitsMap[posH + i][posL + i].GetUnit().health <= 0)
					{
						DropInventory(UnitsMap[posH + i][posL + i]);
						UnitsMap[posH + i][posL + i].type = UnitEmpty;
						xp += UnitsMap[posH + i][posL + i].unit.uEnemy.xpIncreasing;
					}
//					sprintf(tmp, "!HP:%i!", UnitsMap[posH + i][posL + i].GetUnit().health);
//					message += tmp;
				}
				move(posH + i, posL + i);
				addch('\\');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
	}
	inventory[ammo_slot].item.invAmmo.count--;
	if(inventory[ammo_slot].item.invAmmo.count <= 0)
	{
		inventory[ammo_slot].type = ItemEmpty;
	}
}

void Enemy::Shoot()
{
	switch(dir)
	{
		case DIR_LEFT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH][posL - i] == 2) break;
				if(UnitsMap[posH][posL - i].type == UnitHero)
				{
					UnitsMap[posH][posL - i].GetUnit().health -= unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus;
					sprintf(tmp, " !%i! ", UnitsMap[posH][posL - i].GetUnit().health);
					message += tmp;
				}
				move(posH, posL - i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_DOWN:
		{	
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH + i][posL] == 2) break;
				if(UnitsMap[posH + i][posL].type == UnitHero)
				{
					UnitsMap[posH + i][posL].GetUnit().health -= unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus;
					sprintf(tmp, " !%i! ", UnitsMap[posH + i][posL].GetUnit().health);
					message += tmp;
				}
				move(posH + i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_UP:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH - i][posL] == 2) break;
				if(UnitsMap[posH - i][posL].type == UnitHero)
				{
					UnitsMap[posH - i][posL].GetUnit().health -= unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus;		
					sprintf(tmp, " !%i! ", UnitsMap[posH - i][posL].GetUnit().health);
					message += tmp;
				}
				move(posH - i, posL);
				addch('|');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
		case DIR_RIGHT:
		{
			for(int i = 1; i < unitWeapon->item.invWeapon.range + unitAmmo->item.invAmmo.range; i++)
			{
				if(map[posH][posL + i] == 2) break;
				if(UnitsMap[posH][posL + i].type == UnitHero)
				{
					UnitsMap[posH][posL + i].GetUnit().health -= unitAmmo->item.invAmmo.damage + unitWeapon->item.invWeapon.damageBonus;	
					sprintf(tmp, " !%i! ", UnitsMap[posH][posL + i].GetUnit().health);
					message += tmp;
				}
				move(posH, posL + i);
				addch('-');
				refresh();
				Delay(DELAY / 3);
			}
			break;
		}
	}
	unitAmmo->item.invAmmo.count--;
	if(unitAmmo->item.invAmmo.count <= 0)
	{
		unitAmmo->type = ItemEmpty;
	}
}

void Hero::mHLogic(int& a1, int& a2)
{
	if(map[posH + a1][posL + a2] != 2 || (map[posH + a1][posL + a2] == 2 && CanHeroMoveThroughWalls) && (posH + a1 > 0 && posH + a1 < Height - 1 && posL + a2 > 0 && posL + a2 < Length - 1))
	{
		if(UnitsMap[posH + a1][posL + a2].type == UnitEmpty)
		{
			PossibleUnit buffer = UnitsMap[posH][posL];
			UnitsMap[posH][posL].type = UnitEmpty;
			posH += a1;
			posL += a2;
			UnitsMap[posH][posL] = buffer;
		}
		else if(UnitsMap[posH + a1][posL + a2].type == UnitEnemy)
		{
			AttackEnemy(a1, a2);
		}
	}
	else if(map[posH + a1][posL + a2] == 2)
	{
		if(heroWeapon->type == ItemTools)
		{
			if(heroWeapon->item.invTools.possibility == 1)
			{
				move(0, Length + 10);
				printw("Do you want to dig this wall (y or n)? ");
				char inpChar = getch();
				if(inpChar == 'y' || inpChar == 'Y')
				{
					map[posH + a1][posL + a2] = 1;
					heroWeapon->item.invTools.uses--;
					if(heroWeapon->item.invTools.uses <= 0)
					{
						sprintf(tmp, "Your %s is broken. ", heroWeapon->GetItem().GetName());
						message += tmp;
						heroWeapon->type = ItemEmpty;
						FindVisibleArray();
					}
					return;
				}
			}
		}
		message += "The wall is the way. ";
		Stop = true;
	}
	FindVisibleArray();
}

bool CheckHeroVisibility(PossibleUnit& unit)
{
//	if(unit.unit.uEnemy.CanSeeCell(hero.posH, hero.posL)) return true;
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH + i][unit.GetUnit().posL] == 2) break;
		if(UnitsMap[unit.GetUnit().posH + i][unit.GetUnit().posL].type == UnitHero)
		{
			unit.unit.uEnemy.dir = DIR_DOWN;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH][unit.GetUnit().posL + i] == 2) break;
		if(UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL + i].type == UnitHero)
		{
			unit.unit.uEnemy.dir = DIR_RIGHT;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH - i][unit.GetUnit().posL] == 2) break;
		if(UnitsMap[unit.GetUnit().posH - i][unit.GetUnit().posL].type == UnitHero)
		{
			unit.unit.uEnemy.dir = DIR_UP;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	for(int i = 1; i < unit.unit.uEnemy.vision; i++)
	{
		if(map[unit.GetUnit().posH][unit.GetUnit().posL - i] == 2) break;
		if(UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL - i].type == UnitHero)
		{
			unit.unit.uEnemy.dir = DIR_LEFT;
			unit.unit.uEnemy.dist = i;
			return true;
		}
	}
	return false;
}

void GetRandDir(PossibleUnit& unit)
{

	int posH = unit.GetUnit().posH, posL = unit.GetUnit().posL;
	unit.unit.uEnemy.dist = 0;
	do
	{
		unit.unit.uEnemy.dir = rand() % 4;
	//	unit.unit.uEnemy.dist = rand() % unit.unit.uEnemy.vision;
	// !COMMENT!			// Go to end of dist, until end ot wall meeting and change or not dist
		switch(unit.unit.uEnemy.dir)
		{
			case DIR_LEFT:
			{
				for(int i = 1; i < unit.unit.uEnemy.vision; i++)
				{
					if(map[posH][posL - i] == 2 || UnitsMap[posH][posL - i].type != UnitEmpty) break;
					unit.unit.uEnemy.dist++;
				}
				break;
			}
			case DIR_UP:
			{
				for(int i = 1; i < unit.unit.uEnemy.vision; i++)
				{
					if(map[posH - i][posL] == 2 || UnitsMap[posH - i][posL].type != UnitEmpty) break;
					unit.unit.uEnemy.dist++;
				}
				break;
			}
			case DIR_DOWN:
			{
				for(int i = 1; i < unit.unit.uEnemy.vision; i++)
				{
					if(map[posH + i][posL] == 2 || UnitsMap[posH + i][posL].type != UnitEmpty) break;
					unit.unit.uEnemy.dist++;
				}
				break;
			}
			case DIR_RIGHT:
			{
				for(int i = 1; i < unit.unit.uEnemy.vision; i++)
				{
					if(map[posH][posL + i] == 2 || UnitsMap[posH][posL + i].type != UnitEmpty) break;
					unit.unit.uEnemy.dist++;
				}
				break;
			}

		}
	}
	while( !unit.unit.uEnemy.dist );
}

void CheckDestinationCell(PossibleUnit& unit, int a1, int a2)
{
	if(UnitsMap[unit.GetUnit().posH + a1][unit.GetUnit().posL + a2].type == UnitHero)
	{
		if(unit.GetUnit().unitWeapon->type == ItemWeapon)
		{
			hero.health -= unit.GetUnit().unitWeapon->item.invWeapon.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
		}
		else if(unit.GetUnit().unitWeapon->type == ItemTools)
		{
			hero.health -= unit.GetUnit().unitWeapon->item.invTools.damage * ( ( 100 - hero.heroArmor->item.invArmor.defence ) / 100.0);
		}
	}
	else if(UnitsMap[unit.GetUnit().posH + a1][unit.GetUnit().posL + a2].type == UnitEmpty)
	{
		unit.unit.uEnemy.dist--;
		unit.GetUnit().posH += a1;
		unit.GetUnit().posL += a2;
		UnitsMap[unit.GetUnit().posH][unit.GetUnit().posL] = unit;
		UnitsMap[unit.GetUnit().posH - a1][unit.GetUnit().posL - a2].type = UnitEmpty;
	}
	else if(UnitsMap[unit.GetUnit().posH + a1][unit.GetUnit().posL + a2].type == UnitEnemy)
	{
		GetRandDir(unit);
	}
}

void UpdatePosition(PossibleUnit& unit)
{
	bool HeroVisible = CheckHeroVisibility(unit);
	if(HeroVisible && unit.unit.uEnemy.unitWeapon->item.invWeapon.Ranged == true && unit.unit.uEnemy.unitAmmo->item.invAmmo.count > 0 && unit.unit.uEnemy.unitWeapon->item.invWeapon.range >= unit.unit.uEnemy.dist)
	{
		unit.unit.uEnemy.Shoot();
		return;
	}
	if(HeroVisible == false && unit.unit.uEnemy.dist <= 0)
	{
		GetRandDir(unit);
	}
	else
	{
		switch(unit.unit.uEnemy.dir)
		{
			case DIR_LEFT:
			{
				CheckDestinationCell(unit, 0, -1);
				break;
			}
			case DIR_DOWN:
			{
				CheckDestinationCell(unit, 1, 0);
				break;
			}
			case DIR_UP:
			{
				CheckDestinationCell(unit, -1, 0);
				break;
			}
			case DIR_RIGHT:
			{
				CheckDestinationCell(unit, 0, 1);
				break;
			}
		}
	}
}

void UpdateAI()
{
	for(int i = 0; i < Height; i++)
	{
		for(int j = 0; j < Length; j++)
		{
			if(UnitsMap[i][j].type == UnitEnemy && UnitsMap[i][j].unit.uEnemy.movedOnTurn != turns)
			{
#				ifdef DEBUG
				sprintf(tmp, "{%i|%i|%i|%i}", i, j, UnitsMap[i][j].unit.uEnemy.symbol, UnitsMap[i][j].GetUnit().health);
				message += tmp;
#				endif
				UnitsMap[i][j].unit.uEnemy.movedOnTurn = turns;
				UpdatePosition(UnitsMap[i][j]);
			}
		}
	}
}

#define FOODSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;
#define ARMORSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;
#define WEAPONSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;
#define AMMOSETTER buffer.posH=h;buffer.posL=l;ItemsMap[h][l][rand()%Depth]=buffer;

void SetItems()
{

	for(int i = 0; i < FOODCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfFood;
			Food buffer;
			buffer = differentFood[p];
			FOODSETTER
		}
		else i--;

	}

	for(int i = 0; i < ARMORCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfArmor;
			Armor buffer;
			buffer = differentArmor[p];
			ARMORSETTER
		}
		else i--;
	}
	
	for(int i = 0; i < WEAPONCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		
		Weapon buffer;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfWeapon;
			buffer = differentWeapon[p];
			WEAPONSETTER
		}
		else i--;
	}

	for(int i = 0; i < AMMOCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		
		Ammo buffer;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfAmmo;
			buffer = differentAmmo[p];
			buffer.count = (rand() % Luck) / 2 + 1;
			AMMOSETTER
		}
		else i--;
	}

	for(int i = 0; i < SCROLLCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		Scroll buffer;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfScroll;
			buffer = differentScroll[p];
			ItemsMap[h][l][rand() % Depth] = buffer;
		}
	}
	for(int i = 0; i < POTIONCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		Potion buffer;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfScroll;
			buffer = differentPotion[p];
			ItemsMap[h][l][rand() % Depth] = buffer;
		}
	}
	for(int i = 0; i < TOOLSCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;

		Tools buffer;

		if(map[h][l] == 1)
		{
			int p = rand() % TypesOfTools;
			buffer = differentTools[p];
			buffer.uses = rand() % Luck + 1;
			ItemsMap[h][l][rand() % Depth] = buffer;
		}
	}
}

void SpawnUnits()
{
	for(int i = 0; i < 1; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		if(map[h][l] == 1 && UnitsMap[h][l].type == UnitEmpty)
		{
			UnitsMap[h][l] = hero;
			hero.posH = h;
			hero.posL = l;
			break;
		}
		else i--;
	}
	for(int i = 0; i < ENEMIESCOUNT; i++)
	{
		int h = rand() % Height;
		int l = rand() % Length;
		if(map[h][l] == 1 && UnitsMap[h][l].type == UnitEmpty)
		{
			int p = rand() % TypesOfEnemies;
			UnitsMap[h][l] = differentEnemies[p];
			UnitsMap[h][l].GetUnit().posH = h;
			UnitsMap[h][l].GetUnit().posL = l;
//			sprintf(tmp, ">%i<", UnitsMap[h][l].GetUnit().unitInventory[0].GetItem().symbol);
//			message += tmp;
		}
		else i--;
	}
}

#ifdef DEBUG

void Draw(){
	
	move(0, 0);

	static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			mapSaved[i][j] = map[i][j];
		}
	}
	
	for(int i = 0; i < Height; i++){
		
		for(int j = 0; j < Length; j++){
			
//			printw("% i ", map[i][j]);										// !DEBUG!
			if(mapSaved[i][j] != 0)
			{
				bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
				
/* Here */			if(hero.FindElementsNumberUnderThisCell(i, j) == 0 && UnitsMap[i][j].type == UnitEmpty)
				{
					switch(mapSaved[i][j])
					{
						case 1:
							if(near)
							{
								addch('_');
							}
							else
							{
								addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							}
							break;
						case 2:
							if(near)
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							}
							else
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK));
							}
							break;
					}
				}
/* Here */			else if(hero.FindElementsNumberUnderThisCell(i, j) == 1 && UnitsMap[i][j].type == UnitEmpty)
				{
					int MeetedElement = hero.FindNotEmptyElementUnderThisCell(i, j);
					switch(ItemsMap[i][j][MeetedElement].GetItem().symbol){

						case 100:
							addch('%');
							break;
						case 101:
							addch('%' | COLOR_PAIR(RED_BLACK));
							break;
						case 300:
							addch('&' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 301:
							addch('&' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 400:
							addch('/' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						case 401:
							addch('/' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 402:
							addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							break;
						case 403:
							addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 404:
							addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 450:
							addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
							break;
						case 451:
							addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						case 500:
							addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 600:
							addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
							break;
						case 700:
							addch('\\' | COLOR_PAIR(YELLOW_BLACK));
							break;
					}
				}
/* Here */			else if(hero.FindElementsNumberUnderThisCell(i, j) > 1 && UnitsMap[i][j].type == UnitEmpty)
				{
					addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
				}
/* Here */			if(UnitsMap[i][j].type == UnitHero)
				{
					addch('@' | COLOR_PAIR(GREEN_BLACK));
				}
/* Here */			else if(UnitsMap[i][j].type == UnitEnemy)
				{
/* Here */				switch(UnitsMap[i][j].GetUnit().symbol)
					{
						case 201:
							addch('@' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 202:
							addch('@' | COLOR_PAIR(GREEN_BLACK) | LIGHT);
							break;
						case 203:
							addch('@' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
					}
				}
			}
			else
			{
				addch(' ');	
			}	
					
		}
		
		printw("\n");
		
	}

}

#else

void Draw(){
	
	move(0, 0);

	static int mapSaved[FIELD_ROWS][FIELD_COLS] = {};

	if(MODE == 2 && !hero.IsMapInInventory())
	{
		for(int i = 0; i < FIELD_ROWS; i++)
		{
			for(int j = 0; j < FIELD_COLS; j++)
			{
				mapSaved[i][j] = 0;
			}
		}
	}

	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			if(seenUpdated[i][j])
			{
				int itemsOnCell = hero.FindElementsNumberUnderThisCell(i, j);
				if( itemsOnCell == 0 )
				{
					mapSaved[i][j] = map[i][j];
				}
				else if( itemsOnCell == 1 )
				{
					mapSaved[i][j] = ItemsMap[i][j][hero.FindNotEmptyElementUnderThisCell( i, j )].GetItem().symbol;
				}
				else
				{
					mapSaved[i][j] = 100500; // Magic constant that means 'pile'
				}
			}
		}
	}
	
	for(int i = 0; i < Height; i++){
		
		for(int j = 0; j < Length; j++){
			
//			printw("% i ", map[i][j]);										// !DEBUG!
			if(mapSaved[i][j] != 0)
			{
				bool near = abs(i - hero.posH) <= 1 && abs(j - hero.posL) <= 1;
				if( seenUpdated[i][j] )
				{
					if( UnitsMap[i][j].type == UnitEmpty )
					{
						switch( mapSaved[i][j] )
						{
							case 1:
								if(seenUpdated[i][j])
								{
									addch('_');
								}
								else
								{
									addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								}
								break;
							case 2:
								if(seenUpdated[i][j])
								{
									addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
								}
								else
								{
									addch('#' | COLOR_PAIR(WHITE_BLACK));
								}
								break;
							case 100:
								addch('%');
								break;
							case 101:
								addch('%' | COLOR_PAIR(RED_BLACK));
								break;
							case 300:
								addch('&' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								break;
							case 301:
								addch('&' | COLOR_PAIR(YELLOW_BLACK));
								break;
							case 400:
								addch('/' | COLOR_PAIR(RED_BLACK) | LIGHT);
								break;
							case 401:
								addch('/' | COLOR_PAIR(YELLOW_BLACK));
								break;
							case 402:
								addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
								break;
							case 403:
								addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
								break;	
							case 404:
								addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
								break;
							case 450:
								addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
								break;
							case 451:
								addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
								break;
							case 100500:
								addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
								break;
							case 500:
								addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
								break;
							case 600:
								addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
								break;
							case 700:
								addch('\\' | COLOR_PAIR(YELLOW_BLACK));
								break;
						}
					}
					else
					{
						if(UnitsMap[i][j].type == UnitHero)
						{
							addch('@' | COLOR_PAIR(GREEN_BLACK));
						}
						else if(UnitsMap[i][j].type == UnitEnemy && seenUpdated[i][j])
						{
							switch(UnitsMap[i][j].GetUnit().symbol)
							{
								case 201:
									addch('@' | COLOR_PAIR(YELLOW_BLACK));
									break;
								case 202:
									addch('@' | COLOR_PAIR(GREEN_BLACK) | LIGHT);
									break;
								case 203:
									addch('@' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
									break;							
							}
						}
					}
				}
				else
				{
					switch( mapSaved[i][j] )
					{
						case 1:
							if(near)
							{
								addch('_');
							}
							else
							{
								addch('_' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							}
							break;
						case 2:
							if(near)
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							}
							else
							{
								addch('#' | COLOR_PAIR(WHITE_BLACK));
							}
							break;
						case 100:
							addch('%');
							break;
						case 101:
							addch('%' | COLOR_PAIR(RED_BLACK));
							break;
						case 300:
							addch('&' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 301:
							addch('&' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 400:
							addch('/' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						case 401:
							addch('/' | COLOR_PAIR(YELLOW_BLACK));
							break;
						case 402:
							addch('/' | COLOR_PAIR(WHITE_BLACK) | LIGHT);
							break;
						case 403:
							addch('/' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;	
						case 404:
							addch('/' | COLOR_PAIR(BLACK_BLACK) | LIGHT);
							break;
						case 450:
							addch(',' | COLOR_PAIR(BLACK_BLACK) | LIGHT); 
							break;
						case 451:
							addch(',' | COLOR_PAIR(RED_BLACK) | LIGHT);
							break;
						case 100500:
							addch('^' | COLOR_PAIR(BLACK_WHITE) | LIGHT);
							break;
						case 500:
							addch('~' | COLOR_PAIR(YELLOW_BLACK) | LIGHT);
							break;
						case 600:
							addch('!' | COLOR_PAIR(BLUE_BLACK) | LIGHT);
							break;
						case 700:
							addch('\\' | COLOR_PAIR(YELLOW_BLACK));
							break;
					}
				}
			}
			else
			{
				addch(' ');	
			}	
					
		}
		
		printw("\n");
		
	}

}

#endif

void ClearScreen()
{
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 180; j++)
		{
			move(i, j);
			addch(' ');
		}
	}
}

void mSettingsMode()
{
	int SwitchMode = 1;
	while(1)
	{
		move(0, 0);
		printw("Choose mode");

		move(1, 0);
		if(SwitchMode == 1)
		{
			addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('1');
		printw(" Normal");
		
		move(2, 0);
		if(SwitchMode == 2)
		{
			addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('2');
		printw(" Hard");

		switch(getch())
		{
			case CONTROL_DOWN:
				if(SwitchMode < 2) SwitchMode++;
				break;
			case CONTROL_UP:
				if(SwitchMode > 1) SwitchMode--;
				break;
			case '\033':
				ClearScreen();
				return;
				break;
			case CONTROL_CONFIRM:
				switch(SwitchMode)
				{
					case 1:
						MODE = 1;
						break;
					case 2:
						MODE = 2;
						break;
				}
				break;
		}
		ClearScreen();
	}
}

void mSettingsMap()
{
	ClearScreen();
	move(0, 0);
	printw("Do you want to load map from file?");
	char inpChar = getch();
	if(inpChar == 'y' || inpChar == 'Y')
	{
		GenerateMap = false;
	}
	ClearScreen();
}

void mSettings()
{
	int SwitchSettings = 1;
	ClearScreen();
	while(1)
	{
		move(0, 0);
		printw("Settings");

		move(1, 0);
		if(SwitchSettings == 1)
		{
			addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('1');
		printw(" Mode");

		move(2, 0);
		if(SwitchSettings == 2)
		{
			addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
		}
		else addch('2');
		printw(" Maps");

		switch(getch())
		{
			case CONTROL_DOWN:
				if(SwitchSettings < 2) SwitchSettings ++;
				break;
			case CONTROL_UP:
				if(SwitchSettings > 1) SwitchSettings --;
				break;
			case CONTROL_CONFIRM:
			{
				switch(SwitchSettings)
				{
					case 1:
					{
						mSettingsMode();
						break;
					}
					case 2:
					{
						mSettingsMap();
						break;
					}
				}
				break;
			}
			case '\033':
			{
				MenuCondition = 0;
				ClearScreen();
				return;
				break;
			}
		}
	}
}

void MainMenu()
{
	int Switch = 1;
	while(1)
	{
		if(MenuCondition == 0)
		{
			ClearScreen();
			move(0, 0);
			printw("Welcome to RLRPG /*I fukd bugs*/");

			move(1, 0);
			if(Switch == 1)
			{
				addch('1' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('1');
			printw(" Start game");

			move(2, 0);
			if(Switch == 2)
			{
				addch('2' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('2');
			printw(" Settings");

			move(3, 0);
			if(Switch == 3)
			{
				addch('3' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('3');
			printw(" About");

			move(4, 0);
			if(Switch == 4)
			{
				addch('4' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('4');
			printw(" Help");

			move(5, 0);
			if(Switch == 5)
			{
				addch('5' | COLOR_PAIR(RED_BLACK) | LIGHT);
			}
			else addch('5');
			printw(" Exit");

			switch(getch())
			{
				case CONTROL_DOWN:
					Switch ++;
					if(Switch > 5) Switch = 5;
					break;
				case CONTROL_UP:
					Switch --;
					if(Switch < 1) Switch = 1;
					break;
				case CONTROL_CONFIRM:
					if(Switch == 1)
					{
						MenuCondition = 1;
					}
					if(Switch == 2)
					{
						MenuCondition = 2;
					}
					if(Switch == 5)
					{
						EXIT = true;
						return;
					}
					break;
			}
		}
		else if(MenuCondition == 2)
		{
			mSettings();
		}
		else if(MenuCondition == 1)
		{
			return;
		}
	}
}

void ReadMap()
{
	FILE* file = fopen("map.me", "r");
	for(int i = 0; i < FIELD_ROWS; i++)
	{
		for(int j = 0; j < FIELD_COLS; j++)
		{
			fscanf(file, "%d", &map[i][j]);
		}
	}
	fclose(file);
}

int LEVELUP = hero.level * hero.level + 4/* * hero.level + 8*/;

void GetXP()
{
	if(hero.xp > LEVELUP)
	{
		hero.level++;
		sprintf(tmp, "Now you are level %i. ", hero.level);
		message += tmp;
		MaxInvItemsWeight += MaxInvItemsWeight / 4;
		DEFAULT_HERO_HEALTH += DEFAULT_HERO_HEALTH / 4;
		hero.health = DEFAULT_HERO_HEALTH;
		LEVELUP = hero.level * hero.level + 4/* * hero.level + 8*/;
	}
}

int main()
{
	initscr();
	
	noecho();
	
	start_color();
	
	init_pair(BLACK_BLACK, COLOR_BLACK, COLOR_BLACK);
	init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);
	init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
	init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
	init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);
	init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);
	init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
	init_pair(BLACK_WHITE, COLOR_BLACK, COLOR_WHITE);
	init_pair(BLACK_RED, COLOR_BLACK, COLOR_RED);

	initialize();
	
	MainMenu();
	if(EXIT)
	{ 	
		endwin();
		return 0;
	}

	if(GenerateMap)
	{
		generate_maze();
	}
	else
	{
		ReadMap();
	}

	hero.health = DEFAULT_HERO_HEALTH;
	hero.symbol = 200;
	hero.hunger = 900;
	Luck = (rand() % 20 + rand() % 20) / 2;

	Food Egg(0);
	Food Apple(1);
	differentFood[0] = Egg;
	differentFood[1] = Apple;
	
	Armor ChainChestplate(0);
	Armor LeatherChestplate(1);
	differentArmor[0] = ChainChestplate;
	differentArmor[1] = LeatherChestplate;

	inventory[0].item = LeatherChestplate;
	inventory[0].type = ItemArmor;
	inventory[0].GetItem().inventorySymbol = 'a';
	inventoryVol++;
	hero.heroArmor = &inventory[0];
	hero.heroArmor->GetItem().attribute = 201;
	
	Weapon CopperShortsword(0);
	Weapon BronzeSpear(1);
	Weapon Musket(2);
	Weapon Stick(3);
	Weapon Shotgun(4);
	differentWeapon[0] = CopperShortsword;
	differentWeapon[1] = BronzeSpear;
	differentWeapon[2] = Musket;
	differentWeapon[3] = Stick;
	differentWeapon[4] = Shotgun;
	
	Tools Pickaxe(0);
	differentTools[0] = Pickaxe;

	Ammo SteelBullets(0);
	Ammo ShotgunShells(1);
	differentAmmo[0] = SteelBullets;
	differentAmmo[1] = ShotgunShells;
	
	Scroll MapScroll(0);
	differentScroll[0] = MapScroll;

	Potion BluePotion(0);
	differentPotion[0] = BluePotion;

	Enemy Barbarian(0);
	Enemy Zombie(1);
	Enemy Guardian(2);
	differentEnemies[0] = Barbarian;
	differentEnemies[1] = Zombie;
	differentEnemies[2] = Guardian;

	hero.heroWeapon = &inventory[EMPTY_SLOT];

	SetItems();

	SpawnUnits();

	hero.FindVisibleArray();

	int TurnsCounter = 0;
	
	Draw();
			
	move(Height, 0);
	sprintf(tmp, "HP: %i ", hero.health);
	bar += tmp;
	sprintf(tmp, "Sat: %i ", hero.hunger);
	bar += tmp;
	sprintf(tmp, "Def: %i ", hero.heroArmor->item.invArmor.defence);
	bar += tmp;
	sprintf(tmp, "Dmg: %i ", hero.heroWeapon->item.invWeapon.damage);
	bar += tmp;
	sprintf(tmp, "XP/L: %i/%i ", hero.xp, hero.level);
	bar += tmp;
	sprintf(tmp, "L: %i ", Luck);								// !DEBUG!
	bar += tmp;										//
	bar += "Bul: |";
	for(int i = 0; i < BANDOLIER; i++)
	{
		if(inventory[AMMO_SLOT + i].type != ItemEmpty)
		{
			sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
			bar += tmp;
		}
		else
		{
			bar += "0|";
		}
	}
	if(hero.isBurdened) bar += "Burdened. ";
	printw("%- 190s", bar.c_str());

	move(hero.posH, hero.posL);
	
	while(1)
	{
		if(EXIT)
		{ 	
			refresh();
			endwin();
			return 0;
		}

		message = "";
		bar = "";
	
		move(hero.posH, hero.posL);

		char inp = getch();
	
		hero.moveHero(inp);

		if(!Stop)
		{
			TurnsCounter++;

			if(hero.hunger < 1)
			{
				move(Height + 1, 0);
				message += "You died from starvation. Press any key to exit.";
				printw("%- 190s", message.c_str());
				getch();
				refresh();
				endwin();
				return 0;
			}

			if(hero.health < 1)
			{
				move(Height + 1, 0);
				message += "You died. Press any key to exit.";
				printw("% -190s", message.c_str());
				getch();		
				refresh();
				endwin();
				return 0;
			}

			if(TurnsCounter > 25 && MODE == 1)
			{
				if(hero.health < DEFAULT_HERO_HEALTH)
				{
					hero.health ++;
				}
				TurnsCounter = 0;
			}
			else if(MODE == 1 && TurnsCounter > 100)
			{
				TurnsCounter = 0;
			}

			hero.hunger--;
			
			if(hero.isBurdened) hero.hunger--;

			UpdateAI();
			
			++turns;

			Draw();
			
			move(Height, 0);										//
			sprintf(tmp, "HP: %i ", hero.health);								//
			bar += tmp;											//
			sprintf(tmp, "Sat: %i ", hero.hunger);								//
			bar += tmp;											//
			sprintf(tmp, "Def: %i ", hero.heroArmor->item.invArmor.defence);				// 
			bar += tmp;											//
			sprintf(tmp, "Dmg: %i ", hero.heroWeapon->item.invWeapon.damage);				//
			bar += tmp;											// Condition bar		
			sprintf(tmp, "XP/L: %i/%i ", hero.xp, hero.level);
			bar += tmp;
			sprintf(tmp, "L: %i ", Luck);									// !DEBUG!
			bar += tmp;											// !!
			bar += "Bul: |";
			for(int i = 0; i < BANDOLIER; i++)
			{
				if(inventory[AMMO_SLOT + i].type != ItemEmpty)
				{
					sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
					bar += tmp;
				}	
				else
				{
					bar += "0|";
				}
			}
			if(hero.isBurdened) bar += "Burdened. ";							//
			printw("%- 190s", bar.c_str());									//
		
			if(hero.hunger < 75)
			{	
				bar += "Hungry. ";
			}

			move(Height + 1, 0);
			
			printw("%- 190s", message.c_str());
			
			if(inp == '\033')
			{	
				move(Height, 0);
				printw("Are you sure want to exit?\n");
				char inp = getch();
				if(inp == 'y' || inp == 'Y' || inp == CONTROL_CONFIRM)
				{
					refresh();
					endwin();
					return 0;
				}
			}	
	
			GetXP();

			move(hero.posH, hero.posL);
		}
		else
		{
			move(Height, 0);										//
			sprintf(tmp, "HP: %i ", hero.health);								//
			bar += tmp;											//
			sprintf(tmp, "Sat: %i ", hero.hunger);								//
			bar += tmp;											//
			sprintf(tmp, "Def: %i ", hero.heroArmor->item.invArmor.defence);				// 
			bar += tmp;											//
			sprintf(tmp, "Dmg: %i ", hero.heroWeapon->item.invWeapon.damage);				//
			bar += tmp;											// Condition bar		
			sprintf(tmp, "XP/L: %i/%i ", hero.xp, hero.level);
			bar += tmp;
			sprintf(tmp, "L: %i ", Luck);									// !DEBUG!
			bar += tmp;											// !!
			bar += "Bul: |";
			for(int i = 0; i < BANDOLIER; i++)
			{
				if(inventory[AMMO_SLOT + i].type != ItemEmpty)
				{
					sprintf(tmp, "%i|", inventory[AMMO_SLOT + i].item.invAmmo.count);
					bar += tmp;
				}	
				else
				{
					bar += "0|";
				}
			}
			if(hero.isBurdened) bar += "Burdened. ";							//
			printw("%- 190s", bar.c_str());									//
		
			if(hero.hunger < 75)
			{	
				bar += "Hungry. ";
			}

			move(Height + 1, 0);
			
			printw("%- 190s", message.c_str());

			Stop = false;
		}
	}
		
	refresh();
		
	endwin();
		
	return 0;
	
}
