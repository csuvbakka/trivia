#include "Game.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

constexpr std::string_view ToStringView(Category category)
{
  switch (category) {
    case Category::Pop: return "Pop";
    case Category::Science: return "Science";
    case Category::Sports: return "Sports";
    case Category::Rock: return "Rock";
  }
  throw std::runtime_error("Unknown category.");
}

Game::Game(std::vector<Player> players) : players(std::move(players)), currentPlayer(0)
{
  for (int i = 0; i < 50; i++) {
    questions[Category::Pop].emplace_back("Pop Question " + std::to_string(i));
    questions[Category::Science].emplace_back("Science Question " + std::to_string(i));
    questions[Category::Sports].emplace_back("Sports Question " + std::to_string(i));
    questions[Category::Rock].emplace_back("Rock Question " + std::to_string(i));
  }
}

Game Game::Create(std::vector<std::string> playerNames)
{
  std::vector<Player> players;
  players.reserve(playerNames.size());

  for (auto&& name : playerNames) {
    players.emplace_back(std::move(name), PlayerState{0, 0, false});
    std::cout << players.back().name << " was added" << std::endl;
    std::cout << "They are player number " << players.size() << std::endl;
  }

  return Game(std::move(players));
}

bool Game::isPlayable()
{
  return (players.size() >= 2);
}

void Game::roll(int roll)
{
  std::cout << players[currentPlayer].name << " is the current player" << std::endl;
  std::cout << "They have rolled a " << roll << std::endl;

  if (players[currentPlayer].state.inPenaltyBox) {
    if (roll % 2 != 0) {
      isGettingOutOfPenaltyBox = true;
      std::cout << players[currentPlayer].name << " is getting out of the penalty box" << std::endl;
    } else {
      isGettingOutOfPenaltyBox = false;
      std::cout << players[currentPlayer].name << " is not getting out of the penalty box"
                << std::endl;
      return;
    }
  }

  movePlayer(roll);
  std::cout << players[currentPlayer].name << "'s new location is "
            << players[currentPlayer].state.place << std::endl;
  std::cout << "The category is " << ToStringView(currentCategory()) << std::endl;
  askQuestion();
}

void Game::movePlayer(int n_steps)
{
  players[currentPlayer].state.place = (players[currentPlayer].state.place + n_steps) % 12;
}

void Game::askQuestion()
{
  auto& questionGroup = questions[currentCategory()];
  std::cout << questionGroup.front() << std::endl;
  questionGroup.pop_front();
}

Category Game::currentCategory()
{
  switch (players[currentPlayer].state.place % 4) {
    case 0: return Category::Pop;
    case 1: return Category::Science;
    case 2: return Category::Sports;
    default: return Category::Rock;
  }
}

bool Game::wasCorrectlyAnswered()
{
  if (players[currentPlayer].state.inPenaltyBox) {
    if (isGettingOutOfPenaltyBox) {
      std::cout << "Answer was correct!!!!" << std::endl;
    } else {
      makeNextPlayerTheCurrent();
      return true;
    }
  } else {
    std::cout << "Answer was corrent!!!!" << std::endl;
  }
  players[currentPlayer].state.purse++;
  std::cout << players[currentPlayer].name << " now has " << players[currentPlayer].state.purse
            << " Gold Coins." << std::endl;

  bool winner = didPlayerWin();
  makeNextPlayerTheCurrent();

  return winner;
}

void Game::makeNextPlayerTheCurrent()
{
  currentPlayer = (currentPlayer + 1) % players.size();
}

bool Game::wrongAnswer()
{
  std::cout << "Question was incorrectly answered" << std::endl;
  std::cout << players[currentPlayer].name + " was sent to the penalty box" << std::endl;
  players[currentPlayer].state.inPenaltyBox = true;

  makeNextPlayerTheCurrent();
  return true;
}

bool Game::didPlayerWin()
{
  return !(players[currentPlayer].state.purse == 6);
}
