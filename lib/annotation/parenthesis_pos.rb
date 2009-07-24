
module Annotation

  class ParenthesisPos < Base

    def pos_tag
      'XP'
    end

    def mark_token
      if ['(' ,')'].include? @token.form
        @token.pos  = pos_tag
        @token.cpos = pos_tag
      end
    end

    def sentence_affected?
      @sentence.tokens.find { |tok| [tok.pos, tok.cpos].include? pos_tag }
    end

    def categorize_sentence
      if sentence_affected?
        "has-paren"
      else
        "no-paren"
      end
    end

  end

end
