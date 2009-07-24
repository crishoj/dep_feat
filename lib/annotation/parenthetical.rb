
module Annotation
  # Extends Annotation::Parenthesis to also mark parentheticals in dashes
  class Parenthetical < Parenthesis

    protected

    def opening?
      if @token.form == '('
        @closing = ')'
        true
      elsif @token.form == '-' and @sentence.forms.count('-') == 2
        @closing = '-'
        true
      end
    end

    def closing?
      @token.form == @closing
    end

  end
end
