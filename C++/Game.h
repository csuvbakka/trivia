#pragma once

#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>


enum class Category { Pop, Science, Sports, Rock };
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

struct Question {
  std::string text;
  Category category;
};
struct Answer {
};

class GameTurn {
 public:
  virtual ~GameTurn() = default;

  virtual int rollDice() const                    = 0;
  virtual std::optional<int> movePlayer(int roll) = 0;
  virtual Question readQuestion(int location)     = 0;
  virtual Answer askQuestion(Question)            = 0;
  virtual bool isAnswerCorrect(Answer)            = 0;
  virtual void onCorrectAnswer()                  = 0;
  virtual void onIncorrectAnswer()                = 0;
};

class Game {
 public:
  Game(int nPlayers);
  virtual ~Game() = default;

  void run();

 private:
  int getNextPlayerId(int currentPlayerId) const;

  virtual std::unique_ptr<GameTurn> newTurn(int playerId) = 0;
  virtual bool didPlayerWin(int playerId) const           = 0;

  int nPlayers_;
};
