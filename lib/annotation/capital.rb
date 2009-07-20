require 'unichars'

module Annotation
  class Capital < Base

    def feature
      'capital'
    end

    def mark_token
      if alphabetical_token? and capitalized_token?
        @token.features << feature
      end
    end

    protected

    def alphabetical_token?
      # Does lowercase edition of the token differ from the uppercase edition?
      Unichars.new(@token.form).downcase != Unichars.new(@token.form).upcase
    end

    def capitalized_token?
      first_letter = Unichars.new(@token.form.chars.first)
      first_letter == first_letter.upcase
    end

  end
end
