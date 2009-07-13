
module Annotation
  class Suffix < Annotator

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
