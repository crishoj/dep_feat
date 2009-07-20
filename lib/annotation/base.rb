
module Annotation
  class Base

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
          yield @token.to_s if block_given?
        end
        # blank line between sentences
        yield '' if block_given?
      end
      @corpus
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

    # Categorize a sentence according to presence of the feature handled by the
    # annotator class
    def categorize_sentence
      if sentence_affected?
        "has-#{self.feature}"
      else
        "no-#{self.feature}"
      end
    end

    def sentence_affected?
      @sentence.tokens.find { |tok| tok.features.include? self.feature }
    end
  
  end
end