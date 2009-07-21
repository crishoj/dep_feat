require 'unichars'

module Annotation
  class Capital < Base

    def feature
      'capital'
    end

    def pre_sentence
      @first_alphabetical = true
    end

    def mark_token
      if alphabetical_token?
        unless @first_alphabetical
          @token.features << feature if capitalized_token?
        end
        @first_alphabetical = false
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
