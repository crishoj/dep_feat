
module Annotation
  class Colon < Span

    def feature
      'colon'
    end

    def pre_sentence
      @in_paren       = false
      @colon          = false
      @colon_in_paren = false
    end

    def mark_token
      case @token.form
      when '('
        @in_paren = true
      when ')'
        @in_paren = false
        @colon_in_paren = false
      end
      @token.features << feature if (@colon or @colon_in_paren)
      if @token.form == ':'
        if @in_paren
          @colon_in_paren = true
        else
          @colon = true
        end
      end
    end

  end
end
