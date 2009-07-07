
require 'lib/annotation'
require 'lib/conll/corpus'

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
        evaluate_token
      end
    end
    @counts
  end

  def categorize(corpus)
    parts = Hash.new { |hash, cat| hash[cat] = Conll::Corpus.new }
    for @sentence in @corpus.sentences
      # Use gold corpus for categorization, as features are missing in the parser output
      category = categorize_sentence
      parts[category] << corpus.sentences[@sentence.index]
    end
    parts
  end

  protected

  def pre_sentence
    # allow subclasses to do work here
  end

  def mark_token
    # allow subclasses to do work here
  end

  def post_sentence
    # allow subclasses to do work here
  end

  def evaluate_token
    # allow subclasses to do work here
  end

  # Categorize a sentence according to presence of the feature handled by the
  # annotator class
  def categorize_sentence
    # allow subclasses to do work here
  end

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

