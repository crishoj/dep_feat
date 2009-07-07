# encoding: UTF-8

module Annotation
  class SpanAnnotator < Annotator

    def evaluate_token
      if token_inside?
        count_token(:inside)
      else
        count_token(:outside)
      end
      # Check dependencies crossing a boundary of the span
      if dep_crossing?
        count_token(:crossing)
      else
        count_token(:non_crossing)
      end
    end

    # Does the dependency of this token cross a boundary of a marked span?
    # E.g. for a parenthesized token: Is the head outside of the parenthesis?
    def dep_crossing?
      system_head_idx = @token.head.index
      # Look for the feature on the token from the gold corpus,
      # as the MSTParser does not preserve features in its output.
      head = @gold_sentence.tokens[system_head_idx]
      if @gold_token.features.include? feature
        not head.features.include? feature
      else
        head.features.include? feature
      end
    end

    # Is the token inside of the span marked by this annotator?
    # E.g. for parenthesis: Is the token parenthesized?
    def token_inside?
      @gold_token.features.include? feature
    end

    protected

    def mark_span(first, last)
      last = (@sentence.tokens.size-1) if last == :end
      @sentence.tokens[first..last].each do |token|
        token.features << feature
      end
    end

    def sentence_affected?
      @sentence.tokens.find { |tok| tok.features.include? self.feature }
    end

    def categorize_sentence
      if sentence_affected?
        "has-#{self.feature}"
      else
        "no-#{self.feature}"
      end
    end

  end
end
