
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

  def pre_sentence
    # allow subclasses to do work here
  end

  def mark_token
    # allow subclasses to do work here
  end

  def post_sentence
    # allow subclasses to do work here
  end

end

