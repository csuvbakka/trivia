#include "Game.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

Game::Game() : places{}, purses{}, currentPlayer(0)
{
  for (int i = 0; i < 50; i++) {
    popQuestions.emplace_back("Pop Question " + std::to_string(i));
    scienceQuestions.emplace_back("Science Question " + std::to_string(i));
    sportsQuestions.emplace_back("Sports Question " + std::to_string(i));
    rockQuestions.emplace_back("Rock Question " + std::to_string(i));
  }
}

bool Game::isPlayable()
{
  return (players.size() >= 2);
}

bool Game::add(std::string playerName)
{
  players.push_back(playerName);
  places[players.size()]       = 0;
  purses[players.size()]       = 0;
  inPenaltyBox[players.size()] = false;

  std::cout << playerName << " was added" << std::endl;
  std::cout << "They are player number " << players.size() << std::endl;
  return true;
}

void Game::roll(int roll)
{
  std::cout << players[currentPlayer] << " is the current player" << std::endl;
  std::cout << "They have rolled a " << roll << std::endl;

  if (inPenaltyBox[currentPlayer]) {
    if (roll % 2 != 0) {
      isGettingOutOfPenaltyBox = true;
      std::cout << players[currentPlayer] << " is getting out of the penalty box" << std::endl;
    } else {
      isGettingOutOfPenaltyBox = false;
      std::cout << players[currentPlayer] << " is not getting out of the penalty box" << std::endl;
      return;
    }
  }

  movePlayer(roll);
  std::cout << players[currentPlayer] << "'s new location is " << places[currentPlayer]
            << std::endl;
  std::cout << "The category is " << currentCategory() << std::endl;
  askQuestion();
}

void Game::movePlayer(int roll)
{
  places[currentPlayer] = (places[currentPlayer] + roll) % 12;
}

void Game::askQuestion()
{
  if (currentCategory() == "Pop") {
    std::cout << popQuestions.front() << std::endl;
    popQuestions.pop_front();
  }
  if (currentCategory() == "Science") {
    std::cout << scienceQuestions.front() << std::endl;
    scienceQuestions.pop_front();
  }
  if (currentCategory() == "Sports") {
    std::cout << sportsQuestions.front() << std::endl;
    sportsQuestions.pop_front();
  }
  if (currentCategory() == "Rock") {
    std::cout << rockQuestions.front() << std::endl;
    rockQuestions.pop_front();
  }
}

std::string Game::currentCategory()
{
  if (places[currentPlayer] == 0)
    return "Pop";
  if (places[currentPlayer] == 4)
    return "Pop";
  if (places[currentPlayer] == 8)
    return "Pop";
  if (places[currentPlayer] == 1)
    return "Science";
  if (places[currentPlayer] == 5)
    return "Science";
  if (places[currentPlayer] == 9)
    return "Science";
  if (places[currentPlayer] == 2)
    return "Sports";
  if (places[currentPlayer] == 6)
    return "Sports";
  if (places[currentPlayer] == 10)
    return "Sports";
  return "Rock";
}

bool Game::wasCorrectlyAnswered()
{
  if (inPenaltyBox[currentPlayer]) {
    if (isGettingOutOfPenaltyBox) {
      std::cout << "Answer was correct!!!!" << std::endl;
      purses[currentPlayer]++;
      std::cout << players[currentPlayer] << " now has " << purses[currentPlayer] << " Gold Coins."
                << std::endl;

      bool winner = didPlayerWin();
      makeNextPlayerTheCurrent();

      return winner;
    } else {
      makeNextPlayerTheCurrent();
      return true;
    }

  } else {
    std::cout << "Answer was corrent!!!!" << std::endl;
    purses[currentPlayer]++;
    std::cout << players[currentPlayer] << " now has " << purses[currentPlayer] << " Gold Coins."
              << std::endl;

    bool winner = didPlayerWin();
    makeNextPlayerTheCurrent();

    return winner;
  }
}

void Game::makeNextPlayerTheCurrent()
{
  currentPlayer++;
  if (currentPlayer == players.size())
    currentPlayer = 0;
}

bool Game::wrongAnswer()
{
  std::cout << "Question was incorrectly answered" << std::endl;
  std::cout << players[currentPlayer] + " was sent to the penalty box" << std::endl;
  inPenaltyBox[currentPlayer] = true;

  makeNextPlayerTheCurrent();
  return true;
}

bool Game::didPlayerWin()
{
  return !(purses[currentPlayer] == 6);
}
