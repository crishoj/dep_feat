
module Annotation
  class Suffix < Base

    def feature
      'suffix'
    end

    def mark_token
      if @token.form =~ /^-.+/
        @token.features << feature
      end
    end

  end
end
