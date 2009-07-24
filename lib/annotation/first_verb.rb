
module Annotation
  class FirstVerb < Base

    def feature
      '1st-verb'
    end

    def pre_sentence
      @found = false
    end

    def mark_token
      if not @found
        if @token.cpos == 'V'
          @token.features << feature
          @found = true
        end
      end
    end

  end
end
