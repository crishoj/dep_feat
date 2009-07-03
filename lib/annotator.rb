
require 'lib/annotation'

class Annotator

  def initialize(corpus)
    @corpus = corpus
  end

  def run
    for @sentence in @corpus.sentences
      pre_sentence
      for @token in @sentence.tokens
        mark_token
      end
      post_sentence
      for @token in @sentence.tokens
        yield @token.to_s
      end
      yield # blank line between sentences
    end
  end

  def evaluate(gold)
    @counts = {}
    for @sentence in @corpus.sentences
      @gold_sentence = gold.sentences[@sentence.index]
      for @token in @sentence.tokens
        @gold_token = @gold_sentence.tokens[@token.index]
        count_token(:total)
        count_token(:crossing) if dep_crossing?
        if token_inside?
          count_token(:inside)
        else
          count_token(:outside)
        end
      end
    end
    @counts
  end

  def pre_sentence
    # allow subclasses to do work here
  end

  def mark_token
    # allow subclasses to do work here
  end

  def post_sentence
    # allow subclasses to do work here
  end

  protected

  def head_correct?
    @token.head.id == @gold_token.head.id
  end

  def label_correct?
    @token.deprel == @gold_token.deprel
  end

  def count_token(category)
    @counts[category] ||= Hash.new(0)
    @counts[category][:tokens] += 1
    if head_correct?
      @counts[category][:head_correct] += 1
      @counts[category][:both_correct] += 1 if label_correct?
    end
  end
  
end

