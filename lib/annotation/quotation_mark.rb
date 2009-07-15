
module Annotation

  class QuotationMark < Base

    def mark_token
      if @token.form == '"'
        @token.pos = 'XQ'
        @token.cpos = 'XQ'
      end
    end

    def sentence_affected?
      @sentence.tokens.find { |tok| tok.pos == 'XQ' or tok.cpos == 'XQ'}
    end

    def categorize_sentence
      if sentence_affected?
        "has-quotes"
      else
        "no-quotes"
      end
    end

  end

end
