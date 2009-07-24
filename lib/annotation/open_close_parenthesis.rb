
module Annotation
  class OpenCloseParenthesis < Parenthesis

    def mark_token
      if @token.form == '('
        @token.features << 'opening'
        @in_parenthesis = true
      elsif @token.form == ')'
        @token.features << 'closing'
        @in_parenthesis = false
      else
        @token.features << feature if @in_parenthesis
      end
    end

  end
end
