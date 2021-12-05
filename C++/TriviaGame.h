#pragma once

#include "Game.h"

#include <list>
#include <ostream>
#include <vector>
#include <string>
#include <unordered_map>

struct TriviaPlayer {
  struct State {
    State(int field, int coins, bool inPenaltyBox)
        : field(field), coins(coins), inPenaltyBox(inPenaltyBox)
    {
    }
    int field;
    int coins;
    bool inPenaltyBox;
  };

  TriviaPlayer(std::string name, State state) : name(std::move(name)), state(state) {}
  std::string name;
  State state;
};
using TriviaQuestionPool = std::unordered_map<Category, std::list<std::string>>;

///  Implements the mechanics of a single turn of the Trivia game.
class TriviaGameTurn : public GameTurn {
 public:
  TriviaGameTurn(TriviaPlayer& player, TriviaQuestionPool& questionPool, std::ostream& logger);

  virtual int rollDice() const override;
  virtual std::optional<int> movePlayer(int roll) override;
  virtual std::optional<Question> readQuestion(int location) override;
  virtual Answer askQuestion(Question question) override;
  virtual bool isAnswerCorrect(Answer answer) override;
  virtual void onCorrectAnswer() override;
  virtual void onIncorrectAnswer() override;

 private:
  std::optional<std::string> nextQuestion(Category category);

  TriviaPlayer& player_;
  TriviaQuestionPool& questionPool_;
  std::ostream& logger_;
};

class TriviaGame : public Game {
 public:
  static std::optional<TriviaGame> Create(std::vector<std::string> playerNames,
                                          TriviaQuestionPool questionPool,
                                          std::ostream& logger);

 private:
  TriviaGame(std::vector<TriviaPlayer> players,
             TriviaQuestionPool questionPool,
             std::ostream& logger);

  virtual std::unique_ptr<GameTurn> newTurn(int playerId) override;
  virtual bool didPlayerWin(int playerId) const override;

  std::vector<TriviaPlayer> players_;
  TriviaQuestionPool questionPool_;
  std::ostream& logger_;
};
