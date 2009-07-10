
require 'lib/annotation/span'

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
      when ':'
        if @in_paren
          @colon_in_paren = true
        else
          @colon = true
        end
      else
        @token.features << feature if (@colon or @colon_in_paren)
      end
    end

  end
end
