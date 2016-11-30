// rats.cpp

// Portions you are to complete are marked with a TODO: comment.
// We've provided some incorrect return statements (so indicated) just
// to allow this skeleton program to compile and run, albeit incorrectly.
// The first thing you probably want to do is implement the utterly trivial
// functions (marked TRIVIAL).  Then get Arena::display going.  That gives
// you more flexibility in the order you tackle the rest of the functionality.
// As you finish implementing each TODO: item, remove its TODO: comment.

#include <iostream>
#include <string>
#include <random>
#include <utility>
#include <cstdlib>
#include <cctype>
using namespace std;

///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 20;            // max number of rows in the arena
const int MAXCOLS = 20;            // max number of columns in the arena
const int MAXRATS = 100;           // max number of rats allowed

const int NORTH = 0;
const int EAST  = 1;
const int SOUTH = 2;
const int WEST  = 3;
const int NUMDIRS = 4;

const int EMPTY      = 0;
const int HAS_POISON = 1;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////

class Arena;  // This is needed to let the compiler know that Arena is a
// type name, since it's mentioned in the Rat declaration.

class Rat
{
public:
    // Constructor
    Rat(Arena* ap, int r, int c);

    // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;

    // Mutators
    void move();

private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    bool    dead;
    bool    slow;
    int deadturns;

    // TODO: You'll probably find that a rat object needs additional
    // data members to support your implementation of the behavior affected
    // by poison pellets.
};

class Player
{
public:
    // Constructor
    Player(Arena* ap, int r, int c);

    // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;

    // Mutators
    string dropPoisonPellet();
    string move(int dir);
    void   setDead();

private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    bool   m_dead;
};

class Arena
{
public:
    // Constructor/destructor
    Arena(int nRows, int nCols);
    ~Arena();

    // Accessors
    int     rows() const;
    int     cols() const;
    Player* player() const;
    int     ratCount() const;
    int     getCellStatus(int r, int c) const;
    int     numberOfRatsAt(int r, int c) const;
    void    display(string msg) const;

    // Mutators
    void setCellStatus(int r, int c, int status);
    bool addRat(int r, int c);
    bool addPlayer(int r, int c);
    void moveRats();

private:
    int     m_grid[MAXROWS][MAXCOLS];
    int     m_rows;
    int     m_cols;
    Player* m_player;
    Rat*    m_rats[MAXRATS];
    int     m_nRats;
    int     m_turns;

    // Helper functions
    void checkPos(int r, int c) const;
    bool isPosInBounds(int r, int c) const;
};

class Game
{
public:
    // Constructor/destructor
    Game(int rows, int cols, int nRats);
    ~Game();

    // Mutators
    void play();

private:
    Arena* m_arena;

    // Helper functions
    string takePlayerTurn();
};

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////

int randInt(int min, int max);
bool decodeDirection(char ch, int& dir);
bool attemptMove(const Arena& a, int dir, int& r, int& c);
bool recommendMove(const Arena& a, int r, int c, int& bestDir);
void clearScreen();

///////////////////////////////////////////////////////////////////////////
//  Rat implementation
///////////////////////////////////////////////////////////////////////////

Rat::Rat(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** A rat must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "***** Rat created with invalid coordinates (" << r << ","
             << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    slow=false;
    dead=false;
    deadturns=0;
}

int Rat::row() const
{
    return m_row;
}

int Rat::col() const
{
    return m_col;
}

bool Rat::isDead() const
{
    return dead;
}

void Rat::move()
{
    int dir = randInt(0, 3);
    if(slow){
        if(deadturns%2!=0) {
            attemptMove(*m_arena, dir, m_row, m_col);
            if (m_arena->getCellStatus(m_row, m_col) == HAS_POISON) {
                m_arena->setCellStatus(m_row, m_col, EMPTY); //eat the poison
                dead=true;
                cout<<"rat is kill"<<endl;
            }
        }
        deadturns++;
    } else
    {
        attemptMove(*m_arena, dir, m_row, m_col);
        if (m_arena->getCellStatus(m_row, m_col)==HAS_POISON){
            m_arena->setCellStatus(m_row, m_col,EMPTY); //eat the poison
            cout<<"ate poison first time"<<endl;
            slow = true;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
//  Player implementation
///////////////////////////////////////////////////////////////////////////

Player::Player(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** The player must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "**** Player created with invalid coordinates (" << r
             << "," << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_dead = false;
}

int Player::row() const
{
    return m_row;
}

int Player::col() const
{
    return m_col;
}

string Player::dropPoisonPellet()
{
    if (m_arena->getCellStatus(m_row, m_col) == HAS_POISON)
        return "There's already a poison pellet at this spot.";
    m_arena->setCellStatus(m_row, m_col, HAS_POISON);
    return "A poison pellet has been dropped.";
}

string Player::move(int dir)
{
    if(attemptMove(*m_arena, dir, m_row, m_col)){ //yes the player can move
        if(m_arena->numberOfRatsAt(m_row, m_col)!=0){ //there's a rat
            m_dead=true; //the player dies
            return "Player walked into a rat and died.";
        }
        switch(dir){ //output that the player has successfully moved
            case 0:
                return "Player moved north.";
            case 1:
                return "Player moved east.";
            case 2:
                return "Player moved south.";
            case 3:
                return "Player moved west.";
        }
    }else
        return "Player couldn't move; player stands.";
}
// TODO:  Attempt to move the player one step in the indicated
//        get numberofratsat working then it's done


bool Player::isDead() const
{
    if(m_dead){
        return true;
    }
    return false;
}

void Player::setDead()
{
    m_dead = true;
}

///////////////////////////////////////////////////////////////////////////
//  Arena implementation
///////////////////////////////////////////////////////////////////////////

Arena::Arena(int nRows, int nCols)
{
    if (nRows <= 0  ||  nCols <= 0  ||  nRows > MAXROWS  ||  nCols > MAXCOLS)
    {
        cout << "***** Arena created with invalid size " << nRows << " by "
             << nCols << "!" << endl;
        exit(1);
    }
    m_rows = nRows;
    m_cols = nCols;
    m_player = nullptr;
    m_nRats = 0;
    m_turns = 0;
    for (int r = 1; r <= m_rows; r++)
        for (int c = 1; c <= m_cols; c++)
            setCellStatus(r, c, EMPTY);
}

Arena::~Arena()
{
    delete m_player;
    for(int i = 0; i < m_nRats; i++){
        delete m_rats[i];
    }
}

int Arena::rows() const
{
    return m_rows;
}

int Arena::cols() const
{
    return m_cols;
}

Player* Arena::player() const
{
    return m_player;
}

int Arena::ratCount() const //gives you the number of valid pointers in m_rats
{
    return m_nRats;
    /*int rats=0;
    for(int i = 0; i < MAXRATS; i++){
        if(m_rats[i]!=nullptr){
            rats++;
        }
    }
    return rats;*/
}

int Arena::getCellStatus(int r, int c) const
{
    checkPos(r, c);
    return m_grid[r-1][c-1];
}

int Arena::numberOfRatsAt(int r, int c) const
{
    int numrats=0;
    /*for(int i = 0; i < MAXRATS; i++){ //check every single element of m_rats
        if(m_rats[i]!=nullptr)
            if(m_rats[i]->row()==r&&m_rats[i]->col()==c)
                numrats++;
    }*/
    for(int i = 0; i < m_nRats; i++){ //check every single element of m_rats
        if(m_rats[i]->row()==r&&m_rats[i]->col()==c)
            numrats++;
    }
    return numrats;
}

void Arena::display(string msg) const
{
    char displayGrid[MAXROWS][MAXCOLS];
    int r, c;

    // Fill displayGrid with dots (empty) and stars (poison pellets)
    for (r = 1; r <= rows(); r++)
        for (c = 1; c <= cols(); c++)
            displayGrid[r-1][c-1] = (getCellStatus(r,c) == EMPTY ? '.' : '*');

    // Indicate each rat's position
    for (r = 1; r <= rows(); r++){
        for (c = 1; c <= cols(); c++){
            if(numberOfRatsAt(r, c)!=0){
                if(numberOfRatsAt(r, c)==1)
                    displayGrid[r-1][c-1] = 'R';
                else
                    displayGrid[r-1][c-1] = (char)(numberOfRatsAt(r, c)+48); //char conversion
            }
        }//end inner for loop
    }//end outer for loop

    // Indicate player's position
    if (m_player != nullptr)
        displayGrid[m_player->row()-1][m_player->col()-1] = (m_player->isDead() ? 'X' : '@');

    // Draw the grid
    clearScreen();
    for (r = 1; r <= rows(); r++)
    {
        for (c = 1; c <= cols(); c++)
            cout << displayGrid[r-1][c-1];
        cout << endl;
    }
    cout << endl;

    // Write message, rat, and player info
    if (msg != "")
        cout << msg << endl;
    cout << "There are " << ratCount() << " rats remaining." << endl;
    if (m_player == nullptr)
        cout << "There is no player!" << endl;
    else if (m_player->isDead())
        cout << "The player is dead." << endl;
    cout << m_turns << " turns have been taken." << endl;
}

void Arena::setCellStatus(int r, int c, int status)
{
    checkPos(r, c);
    m_grid[r-1][c-1] = status;
}

bool Arena::addRat(int r, int c)
{
    if (! isPosInBounds(r, c))
        return false;

    // Don't add a rat on a spot with a poison pellet
    if (getCellStatus(r, c) != EMPTY)
        return false;

    // Don't add a rat on a spot with a player
    if (m_player != nullptr  &&  m_player->row() == r  &&  m_player->col() == c)
        return false;

    // If there are MAXRATS existing rats, return false.  Otherwise,
    // dynamically allocate a new rat at coordinates (r,c).  Save the
    // pointer to the newly allocated rat and return true.

    // TODO:  check if this is correct
    if(ratCount()>=MAXRATS)
        return false;

    //add a rat, there is a space in the array
    m_rats[m_nRats]=new Rat(this, r, c);
    m_nRats++;
    return true;
    /*
    for(int i = 0; i < MAXRATS; i++){ //FIND AN EMPTY SPACE IN THE ARRAY TO PUT IN THE RAT
        if(m_rats[i]==nullptr){
            m_rats[i]=new Rat(this, r, c);
            return true;
        }
    }
    return false;*/

}

bool Arena::addPlayer(int r, int c)
{
    if (! isPosInBounds(r, c))
        return false;

    // Don't add a player if one already exists
    if (m_player != nullptr)
        return false;

    // Don't add a player on a spot with a rat
    if (numberOfRatsAt(r, c) > 0)
        return false;

    m_player = new Player(this, r, c);
    return true;
}

void Arena::moveRats() //player moves first, and then rats move. this explains why in this turn rat can kill player
{
    // Move all rats blah blah
    // TODO:  Move each rat.  Mark the player as dead if necessary.
    //        Deallocate any dead dynamically allocated rat.
    for(int i = 0; i < m_nRats; i++){
        m_rats[i]->move();
        if(m_rats[i]->col()==m_player->col()&&m_rats[i]->row()==m_player->row()){
            m_player->setDead();
        }
        if(m_rats[i]->isDead()){
            delete m_rats[i];
            for(int j = i+1;j<m_nRats;j++){
                m_rats[j-1]=m_rats[j];
            }
            cout<<"RAT COUNT DECREMENT"<<endl;
            m_nRats--;
        }
    }
    m_turns++;
}

bool Arena::isPosInBounds(int r, int c) const
{
    return (r >= 1  &&  r <= m_rows  &&  c >= 1  &&  c <= m_cols);
}

void Arena::checkPos(int r, int c) const
{
    if (r < 1  ||  r > m_rows  ||  c < 1  ||  c > m_cols)
    {
        cout << "***** " << "Invalid arena position (" << r << ","
             << c << ")" << endl;
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////
//  Game implementation
///////////////////////////////////////////////////////////////////////////

Game::Game(int rows, int cols, int nRats)
{
    if (nRats < 0)
    {
        cout << "***** Cannot create Game with negative number of rats!" << endl;
        exit(1);
    }
    if (nRats > MAXRATS)
    {
        cout << "***** Trying to create Game with " << nRats
             << " rats; only " << MAXRATS << " are allowed!" << endl;
        exit(1);
    }
    if (rows == 1  &&  cols == 1  &&  nRats > 0)
    {
        cout << "***** Cannot create Game with nowhere to place the rats!" << endl;
        exit(1);
    }

    // Create arena
    m_arena = new Arena(rows, cols);

    // Add player
    int rPlayer;
    int cPlayer;
    do
    {
        rPlayer = randInt(1, rows);
        cPlayer = randInt(1, cols);
    } while (m_arena->getCellStatus(rPlayer, cPlayer) != EMPTY);
    m_arena->addPlayer(rPlayer, cPlayer);

    // Populate with rats
    while (nRats > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
        if (r == rPlayer && c == cPlayer)
            continue;
        m_arena->addRat(r, c);
        nRats--;
    }
}

Game::~Game()
{
    delete m_arena;
}

string Game::takePlayerTurn()
{
    for (;;)
    {
        cout << "Your move (n/e/s/w/x or nothing): ";
        string playerMove;
        getline(cin, playerMove);

        Player* player = m_arena->player();
        int dir;

        if (playerMove.size() == 0)
        {
            if (recommendMove(*m_arena, player->row(), player->col(), dir))
                return player->move(dir);
            else
                return player->dropPoisonPellet();
        }
        else if (playerMove.size() == 1)
        {
            if (tolower(playerMove[0]) == 'x')
                return player->dropPoisonPellet();
            else if (decodeDirection(playerMove[0], dir))
                return player->move(dir);
        }
        cout << "Player move must be nothing, or 1 character n/e/s/w/x." << endl;
    }
}

void Game::play()
{
    m_arena->display("");
    while ( ! m_arena->player()->isDead()  &&  m_arena->ratCount() > 0)
    {
        string msg = takePlayerTurn();
        Player* player = m_arena->player();
        if (player->isDead())
        {
            cout << msg << endl;
            break;
        }
        m_arena->moveRats();
        m_arena->display(msg);
    }
    if (m_arena->player()->isDead())
        cout << "You lose." << endl;
    else
        cout << "You win." << endl;
}

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function implementation
///////////////////////////////////////////////////////////////////////////

// Return a uniformly distributed random int from min to max, inclusive
int randInt(int min, int max)
{
    if (max < min)
        swap(max, min);
    static random_device rd;
    static mt19937 generator(rd());
    uniform_int_distribution<> distro(min, max);
    return distro(generator);
}

bool decodeDirection(char ch, int& dir)
{
    switch (tolower(ch))
    {
        default:  return false;
        case 'n': dir = NORTH; break;
        case 'e': dir = EAST;  break;
        case 's': dir = SOUTH; break;
        case 'w': dir = WEST;  break;
    }
    return true;
}

// Return false without changing anything if moving one step from (r,c)
// in the indicated direction would run off the edge of the arena.
// Otherwise, update r and c to the position resulting from the move and
// return true.
bool attemptMove(const Arena& a, int dir, int& r, int& c)
{
    switch (dir)
    {
        case 0:
            if((r-1)>=1){
                r--;
            }else
                return false;
            break;
        case 1:
            if((c+1)<=a.cols()){
                c++;
            }else
                return false;
            break;
        case 2:
            if((r+1)<=a.rows()){
                r++;
            }else
                return false;
            break;
        case 3:
            if((c-1)>=1){
                c--;
            }else
                return false;
            break;
    }
    return true;
}

// Recommend a move for a player at (r,c):  A false return means the
// recommendation is that the player should drop a poison pellet and not
// move; otherwise, this function sets bestDir to the recommended
// direction to move and returns true.
int checkRadius(const Arena& a, int r, int c){
    int points = 0;
    points+=a.numberOfRatsAt(r-1, c);
    points+=a.numberOfRatsAt(r+1, c);
    points+=a.numberOfRatsAt(r, c-1);
    points+=a.numberOfRatsAt(r, c+1);
    return points;
}
bool recommendMove(const Arena& a, int r, int c, int& bestDir)
{
    int minpoints = checkRadius(a, r, c);
    bestDir=4;
    if((r-1)>=1&&checkRadius(a, r-1, c)< minpoints&&a.numberOfRatsAt(r-1, c)==0){
        minpoints = checkRadius(a, r-1, c);
        bestDir=0; //north
    }
    if((c+1)<=a.cols()&&checkRadius(a, r, c+1)< minpoints&&a.numberOfRatsAt(r, c+1)==0){
        minpoints = checkRadius(a, c+1, c);
        bestDir=1;
    }
    if((r+1)<=a.rows()&&checkRadius(a, r+1, c)< minpoints&&a.numberOfRatsAt(r+1, c)==0){
        minpoints = checkRadius(a, r+1, c);
        bestDir=2;
    }
    if((c-1)>=1&&checkRadius(a, r, c-1)< minpoints&&a.numberOfRatsAt(r, c-1)==0){
        minpoints = checkRadius(a, r, c-1);
        bestDir=3;
    }
    if (bestDir==4)
        return false;
    else
        return true;

}
///////////////////////////////////////////////////////////////////////////
// main()
///////////////////////////////////////////////////////////////////////////

void doBasicTests();
int main()
{
    doBasicTests();
    return 0;
    // Create a game
    // Use this instead to create a mini-game:   Game g(3, 5, 2);
    Game g(3, 5, 1);

    // Play the game
    g.play();
}

///////////////////////////////////////////////////////////////////////////
//  clearScreen implementation
///////////////////////////////////////////////////////////////////////////

// DO NOT MODIFY OR REMOVE ANY CODE BETWEEN HERE AND THE END OF THE FILE!!!
// THE CODE IS SUITABLE FOR VISUAL C++, XCODE, AND g++ UNDER LINUX.

// Note to Xcode users:  clearScreen() will just write a newline instead
// of clearing the window if you launch your program from within Xcode.
// That's acceptable.  (The Xcode output window doesn't have the capability
// of being cleared.)

#ifdef _MSC_VER  //  Microsoft Visual C++

#pragma warning(disable : 4005)
#include <windows.h>

void clearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD upperLeft = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
                                                        &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, upperLeft);
}

#else  // not Microsoft Visual C++, so assume UNIX interface

#include <iostream>
#include <cstring>
#include <cstdlib>

void clearScreen()  // will just write a newline in an Xcode output window
{
    static const char* term = getenv("TERM");
    if (term == nullptr  ||  strcmp(term, "dumb") == 0)
        cout << endl;
    else
    {
        static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal esc seq:  ESC [
        cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
    }
}

#endif
#include <type_traits>
#include <cassert>

#define CHECKTYPE(c, f, r, a)  \
	static_assert(std::is_same<decltype(&c::f), r (c::*)a>::value, \
	   "FAILED: You changed the type of " #c "::" #f);  \
	[[gnu::unused]] r (c::* xxx##c##_##f) a = &c::f

void thisFunctionWillNeverBeCalled()
{
    // If the student deleted or changed the interfaces to the public
    // functions, this won't compile.  (This uses magic beyond the scope
    // of CS 31.)

    Rat r(static_cast<Arena*>(0), 1, 1);
    CHECKTYPE(Rat, row, int, () const);
    CHECKTYPE(Rat, col, int, () const);
    CHECKTYPE(Rat, isDead, bool, () const);
    CHECKTYPE(Rat, move, void, ());

    Player p(static_cast<Arena*>(0), 1, 1);
    CHECKTYPE(Player, row, int, () const);
    CHECKTYPE(Player, col, int, () const);
    CHECKTYPE(Player, isDead, bool, () const);
    CHECKTYPE(Player, dropPoisonPellet, string, ());
    CHECKTYPE(Player, move, string, (int));
    CHECKTYPE(Player, setDead, void, ());

    Arena a(1, 1);
    CHECKTYPE(Arena, rows, int, () const);
    CHECKTYPE(Arena, cols, int, () const);
    CHECKTYPE(Arena, player, Player*, () const);
    CHECKTYPE(Arena, ratCount, int, () const);
    CHECKTYPE(Arena, getCellStatus, int, (int,int) const);
    CHECKTYPE(Arena, numberOfRatsAt, int, (int,int) const);
    CHECKTYPE(Arena, display, void, (string) const);
    CHECKTYPE(Arena, setCellStatus, void, (int,int,int));
    CHECKTYPE(Arena, addRat, bool, (int,int));
    CHECKTYPE(Arena, addPlayer, bool, (int,int));
    CHECKTYPE(Arena, moveRats, void, ());

    Game g(1,1,1);
    CHECKTYPE(Game, play, void, ());
}

void findTheRat(const Arena& a, int& r, int& c)
{
    if      (a.numberOfRatsAt(r-1, c) == 1) r--;
    else if (a.numberOfRatsAt(r+1, c) == 1) r++;
    else if (a.numberOfRatsAt(r, c-1) == 1) c--;
    else if (a.numberOfRatsAt(r, c+1) == 1) c++;
    else assert(false);
}

void doBasicTests()
{
    {
        Arena a(10, 20);
        a.addPlayer(2, 5);
        Player* pp = a.player();
        assert(pp->row() == 2  &&  pp->col() == 5  && ! pp->isDead());
        assert(pp->move(NORTH) == "Player moved north.");
        assert(pp->row() == 1  &&  pp->col() == 5  && ! pp->isDead());
        assert(pp->move(NORTH) == "Player couldn't move; player stands.");
        assert(pp->row() == 1  &&  pp->col() == 5  && ! pp->isDead());
        pp->setDead();
        assert(pp->row() == 1  &&  pp->col() == 5  && pp->isDead());
    }
    {
        Arena a(10, 20);
        int r = 4;
        int c = 4;
        a.setCellStatus(r-1, c, HAS_POISON);
        a.setCellStatus(r+1, c, HAS_POISON);
        a.setCellStatus(r, c-1, HAS_POISON);
        a.setCellStatus(r, c+1, HAS_POISON);
        a.addRat(r, c);
        a.addPlayer(8, 18);
        assert(a.ratCount() == 1);
        assert(a.numberOfRatsAt(r, c) == 1);
        a.moveRats();
        assert(a.ratCount() == 1);
        assert(a.numberOfRatsAt(r, c) == 0);
        findTheRat(a, r, c);
        assert(a.getCellStatus(r, c) != HAS_POISON);
        a.moveRats();
        assert(a.ratCount() == 1  &&  a.numberOfRatsAt(r, c) == 1);
        a.moveRats();
        assert(a.ratCount() == 1  &&  a.numberOfRatsAt(r, c) == 0);
        findTheRat(a, r, c);
        a.moveRats();
        assert(a.ratCount() == 1  &&  a.numberOfRatsAt(r, c) == 1);
        a.setCellStatus(r-1, c, HAS_POISON);
        a.setCellStatus(r+1, c, HAS_POISON);
        a.setCellStatus(r, c-1, HAS_POISON);
        a.setCellStatus(r, c+1, HAS_POISON);
        a.moveRats();
        assert(a.ratCount() == 0  &&  a.numberOfRatsAt(r, c) == 0);
        assert(a.numberOfRatsAt(r-1, c) == 0);
        assert(a.numberOfRatsAt(r+1, c) == 0);
        assert(a.numberOfRatsAt(r, c-1) == 0);
        assert(a.numberOfRatsAt(r, c+1) == 0);

        for (int k = 0; k < MAXRATS/4; k++)
        {
            a.addRat(7, 18);
            a.addRat(9, 18);
            a.addRat(8, 17);
            a.addRat(8, 19);
        }
        assert(! a.player()->isDead());
        a.moveRats();
        assert(a.player()->isDead());
    }
    cout << "Passed all basic tests" << endl;
}
