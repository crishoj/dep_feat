# encoding: UTF-8

module Annotation
  class QuotedPhrase < Span

    ENCLOSURES = {
      '"'  => '"',
      '«'  => '»',
      '``' => "''"
    }
    OPENINGS = ENCLOSURES.keys
    CLOSINGS = ENCLOSURES.values

    def feature
      'quote'
    end

    def pre_sentence
      @in_quote = false
    end

    def mark_token
      if @in_quote
        if @token.form == @closing
          unless @token.last? and @from.first?
            # Only annotate embedded quotations
            mark_span(@from, @token)
          end
          @in_quote = false
        elsif @token.last? 
          # End of sentence
          if @from.prev.form == ':'
            # Open-ended quotation
            mark_span(@from, @token)
          end
        end
      elsif OPENINGS.include? @token.form
        @in_quote = true
        @from = @token
        @closing = ENCLOSURES[@token.form]
      end
    end

  end
end