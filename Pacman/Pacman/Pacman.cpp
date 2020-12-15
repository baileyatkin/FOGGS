#include "Pacman.h"

#include <sstream>

#include <time.h>

#include <iostream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanSpeed(0.1f), _cPacmanFrameTime(250), _cMunchieFrameTime(500), _cCherryFrameTime(500)
{
	// Seeds number generator
	srand(time(NULL));

	//Initialise member variables
	_pacman = new Player();
	{
		_pacman->direction = 0;
		_pacman->frame = 0;
		_pacman->currentFrameTime = 0;
		_pacman->speedMultiplier = 1.0f;
		_pacman->dead = false;
	}

	int i;

	for (i = 0; i < GHOSTCOUNT; i++)
	{
		//Initialise ghost character
		_ghosts[i] = new MovingEnemy();
		_ghosts[i]->direction = 1;
		_ghosts[i]->speed = 0.2f;
	}

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i] = new Enemy();
		_munchies[i]->currentFrameTime = rand() % 500 + 50;
		_munchies[i]->frame = rand() % 1;
		_munchies[i]->dead = false;

	}

	_cherry = new Enemy();
	{
		_cherry->frame = 0;
		_cherry->currentFrameTime = 0;
	}

	_paused = false;
	_pKeyDown = false;
	_start = true;
	_pop = new SoundEffect();

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();
	if (!Audio::IsInitialised())
	{
		std::cout << "Audio is nor initialised" << std::endl;
	}

	if (!_pop->IsLoaded())
	{
		std::cout << "_pop meber sound effect has not loaded" << std::endl;
	}

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

Pacman::~Pacman()
{
	delete _pacman->texture;
	delete _pacman->sourceRect;
	delete _pacman->position;
	delete _pacman;

	int nCount = 0;
	for (nCount = 0; nCount < GHOSTCOUNT; nCount++)
	{
		delete _ghosts[nCount]->texture;
		delete _ghosts[nCount]->sourceRect;
		delete _ghosts[nCount]->position;
		delete _ghosts[nCount];
	}
	delete[] _ghosts;

	delete _munchies[0]->texture;

	for (nCount = 0; nCount < MUNCHIECOUNT; nCount++)
	{
		delete _munchies[nCount]->position;
		delete _munchies[nCount]->sourceRect;
		delete _munchies[nCount];
	}
	delete[] _munchies;

	delete _cherry->texture;
	delete _cherry->sourceRect;
	delete _cherry->rect;
	delete _cherry;

	delete _pop;
}

void Pacman::LoadContent()
{
	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->position = new Vector2(350.0f, 350.0f);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Load Munchie
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/MunchieSpriteSheet.png", false);
	int i;

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->texture = new Texture2D();
		_munchies[i]->texture = munchieTex;
		_munchies[i]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
		_munchies[i]->sourceRect = new Rect(12.0f, 12.0f, 12, 12);
	}
	// Load Cherry
	_cherry->texture = new Texture2D();
	_cherry->texture->Load("Textures/CherrySpriteSheet.png", true);
	_cherry->position = new Vector2(0.0f, 0.0f);
	_cherry->rect = new Rect(150.0f, 442.0f, 32, 32);
	_cherry->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	//Initialise ghost character
	Texture2D* ghostTex = new Texture2D();
	ghostTex->Load("Textures/GhostBlue.png", false);
	i = 0;
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i]->texture = new Texture2D();
		_ghosts[i]->texture = ghostTex;
		_ghosts[i]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
	}

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);

	// Set Menu Parameters
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/Transparency.png", false);
	_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	// Load Sound
	_pop->Load("Sounds/pop.wav");
}

void Pacman::Update(int elapsedTime)
{
	//Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();

	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();

	// Updates the pacman sprite to open and close mouth
	_pacman->currentFrameTime += elapsedTime;

	// Updates the Pacman sprite depending on the direction he is going
	_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacman->direction;

	// Start menu, player has to press the space key in order to start the game
	if (_start)
	{
		if (keyboardState->IsKeyDown(Input::Keys::SPACE) && !_spaceKeyDown)
		{
			_spaceKeyDown = true;
			_start = !_start;
		}

		if (keyboardState->IsKeyUp(Input::Keys::SPACE))
			_spaceKeyDown = false;
	}

	// Makes sure player cannot input when in the start menu
	if(!_start)
	{
		CheckPaused(keyboardState, Input::Keys::P);

		// Stops the player inputting instructions when they are in the pause menu
		if (!_paused)
		{
			Input(elapsedTime, keyboardState, mouseState);
			UpdatePacman(elapsedTime);
			int i;
			for (int i = 0; i < GHOSTCOUNT; i++)
			{
				UpdateGhost(_ghosts[i], elapsedTime);
			}
			CheckGhostCollisions();

			CheckMunchieCollisions();
			for (int i = 0; i < MUNCHIECOUNT; i++)
			{
				UpdateMunchies(_munchies[i], elapsedTime);
			}
			UpdateCherry(elapsedTime);
			CheckViewportCollision();
		}
	}
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState)
{
	float pacmanSpeed = _cPacmanSpeed * elapsedTime * _pacman->speedMultiplier;

	// Checks if D key is pressed
	if (keyboardState->IsKeyDown(Input::Keys::D))
	{
		_pacman->position->X += pacmanSpeed; //Moves Pacman across X axis
		_pacman->direction = 0;
	}

	// Checks if S key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::S))
	{
		_pacman->position->Y += pacmanSpeed; //Moves Pacman across Y axis
		_pacman->direction = 1;
	}

	// Checks if A key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::A))
	{
		_pacman->position->X -= pacmanSpeed; //Moves Pacman across X axis
		_pacman->direction = 2;
	}

	// Checks if W key is pressed
	else if (keyboardState->IsKeyDown(Input::Keys::W))
	{
		_pacman->position->Y -= pacmanSpeed; //Moves Pacman across Y axis
		_pacman->direction = 3;
	}

	// Handle Mouse Input - Reposition Cherry
	if (mouseState->LeftButton == Input::ButtonState::PRESSED)
	{
		_cherry->position->X = mouseState->X;
		_cherry->position->Y = mouseState->Y;
	}

	// Randomly move cherry
	if (keyboardState->IsKeyDown(Input::Keys::R) && !_rKeyDown)
	{
		_rKeyDown = true;
		_cherry->position->X = (rand() % Graphics::GetViewportWidth());
		_cherry->position->Y = (rand() % Graphics::GetViewportHeight());
	}

	if (keyboardState->IsKeyUp(Input::Keys::R))
		_rKeyDown = false;

	// Speed Multiplier
	if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT))
	{
		//Apply Multiplier
		_pacman->speedMultiplier = 2.0f;
	}
	else
	{
		//Reset Multiplier
		_pacman->speedMultiplier = 1.0f;
	}
}

void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pausekey)
{
	// Pauses the game
	if (state->IsKeyDown(Input::Keys::P) && !_pKeyDown)
	{
		_pKeyDown = true;
		_paused = !_paused;
	}

	if (state->IsKeyUp(Input::Keys::P))
		_pKeyDown = false;
}

void Pacman::CheckViewportCollision()
{
	// When Pacman goes to the border of 1 side of the screen, he appears on the opposite side
	if (_pacman->position->X > Graphics::GetViewportWidth())
		_pacman->position->X = 0;

	if (_pacman->position->Y > Graphics::GetViewportHeight())
		_pacman->position->Y = 0;

	if (_pacman->position->X + _pacman->sourceRect->Width < 0)
		_pacman->position->X = Graphics::GetViewportWidth();

	if (_pacman->position->Y + _pacman->sourceRect->Width < 0)
		_pacman->position->Y = Graphics::GetViewportHeight();
}

void Pacman::CheckGhostCollisions()
{
	// Local Variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < GHOSTCOUNT; i++)
	{
		//Populate variables with Ghost data
		bottom2 = _ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height;
		left2 = _ghosts[i]->position->X;
		right2 = _ghosts[i]->position->X + _ghosts[i]->sourceRect->Width;
		top2 = _ghosts[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			_pacman->dead = true;
			i = GHOSTCOUNT;
		}
	}
}

void Pacman::CheckMunchieCollisions()
{
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;
	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		bottom2 = _munchies[i]->position->Y + _munchies[i]->sourceRect->Height;
		left2 = _munchies[i]->position->X;
		right2 = _munchies[i]->position->X + _munchies[i]->sourceRect->Width;
		top2 = _munchies[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (!_munchies[i]->dead))
		{
			_munchies[i]->dead = true;
			Audio::Play(_pop);
		}
	}
}

void Pacman::UpdatePacman(int elapsedTime)
{
	if (_pacman->currentFrameTime > _cPacmanFrameTime)
	{
		_pacman->frame++;

		if (_pacman->frame >= 2)
			_pacman->frame = 0;

		_pacman->currentFrameTime = 0;
	}

	_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame;
}

void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	if (ghost->direction == 1)
	{
		ghost->position->X += ghost->speed * elapsedTime;
		ghost->sourceRect->X = 0;
		ghost->sourceRect->Y = 0;
	}
	else if (ghost->direction == 2)
	{
		ghost->position->Y += ghost->speed * elapsedTime;
		ghost->sourceRect->X = 20;
		ghost->sourceRect->Y = 20;
	}
	else if (ghost->direction == 3)
	{
		ghost->position->X -= ghost->speed * elapsedTime;
		ghost->sourceRect->X = 20;
		ghost->sourceRect->Y = 0;
	}
	else if (ghost->direction == 4)
	{
		ghost->position->Y -= ghost->speed * elapsedTime;
		ghost->sourceRect->X = 0;
		ghost->sourceRect->Y = 20;
	}

	if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportWidth()) //Hits right edge
	{
		ghost->direction = 3; //Change Direction
	}
	else if (ghost->position->X <= 0) //Hits left edge
	{
		ghost->direction = 1; //Change Direction
	}
}

void Pacman::UpdateMunchies(Enemy* _munchie, int elapsedTime)
{
	_munchie->currentFrameTime += elapsedTime;

	if (_munchie->currentFrameTime > _cMunchieFrameTime)
	{
		_munchie->frame++;

		if (_munchie->frame >= 2)
			_munchie->frame = 0;

		_munchie->currentFrameTime = 0;
	}

	_munchie->sourceRect->X = _munchie->sourceRect->Width *_munchie->frame;
}

void Pacman::UpdateCherry(int elapsedTime)
{
	_cherry->currentFrameTime += elapsedTime;

	if (_cherry->currentFrameTime > _cCherryFrameTime)
	{
		_cherry->frame++;

		if (_cherry->frame >= 2)
			_cherry->frame = 0;

		_cherry->currentFrameTime = 0;
	}

	_cherry->sourceRect->X = _cherry->sourceRect->Width * _cherry->frame;
}

void Pacman::Draw(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "Pacman X: " << _pacman->position->X << " Y: " << _pacman->position->Y;

	SpriteBatch::BeginDraw(); // Starts Drawing

	if (!_pacman->dead)
	{
		SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect); // Draws Pacman
	}

	int i;

	for (i = 0; i < GHOSTCOUNT; i++)
	{
		SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect);
	}

	for (i = 0; i < MUNCHIECOUNT; i++)
	{
		if (_munchies[i]->frame == 0 && !_munchies[i]->dead)
		{
			// Draws Red Munchie
			SpriteBatch::Draw(_munchies[i]->texture, _munchies[i]->position, _munchies[i]->sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);

		}
		else if (!_munchies[i]->dead)
		{
			// Draw Blue Munchie
			SpriteBatch::Draw(_munchies[i]->texture, _munchies[i]->position, _munchies[i]->sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);
		}
	}

	if (_cherry->frame == 0)
	{
		// Draws Cherry
		SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);

	}
	else
	{
		// Draw inverted Cherry
		SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);
	}
	
	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);

	// Draws the start menu
	if (_start)
	{
		std::stringstream menuStream;
		menuStream << "PRESS SPACE TO START!";

		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	}

	// Draws the pause menu
	if (_paused)
	{
		std::stringstream menuStream;
		menuStream << "PAUSED!";

		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	}

	SpriteBatch::EndDraw(); // Ends Drawing
}