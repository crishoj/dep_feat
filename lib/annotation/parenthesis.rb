
# TODO: Match parentheticals with commas (tokens inside < 5, or train chunker?)

module Annotation
  class Parenthesis < Span

    def feature
      'paren'
    end

    def pre_sentence
      @in_parenthesis = false
    end

    def mark_token
      if @in_parenthesis and closing?
        @in_parenthesis = false
      elsif opening?
        @in_parenthesis = true
      else
        @token.features << feature if @in_parenthesis
      end
    end

    protected

    def opening?
      @token.form == '('
    end
    
    def closing?
      @token.form == ')'
    end

  end
end
