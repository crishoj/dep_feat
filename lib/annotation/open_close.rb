
module Annotation
  class OpenClose < Base

    def feature
      'opening' # just for categorization purposes
    end

    def mark_token
      if opening?
        @token.features << 'opening'
      elsif closing?
        @token.features << 'closing'
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
