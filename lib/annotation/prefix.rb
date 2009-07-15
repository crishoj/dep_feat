
module Annotation
  class Prefix < Base

    def feature
      'prefix'
    end

    def mark_token
      if @token.form =~ /.+-$/
        # Ends with hyphen
        @token.features << feature
      end
    end

  end
end
