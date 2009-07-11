
module Annotation
  class Bullet < Annotator

    LIST_COUNTER  = /^[0-9]+$/ # A numeral 
    BULLET_MARKER = /^[).]$/   # A closing parenthesis or a dot

    def feature
      'bullet'
    end

    def pre_sentence
      if @sentence.tokens[0].form =~ LIST_COUNTER
        if @sentence.tokens[1].form =~ BULLET_MARKER
          @sentence.tokens[0].features << feature
          @sentence.tokens[1].features << feature
        end
      end
    end

  end
end
