
module Annotation
  class Bullet < Annotator

    def list_counter? token
      token.form =~ /.+/ # Anything
    end

    def bullet_marker? token
      token.form =~ /^[).:]$/  # A closing parenthesis, a dot or a colon
    end

    def feature
      'bullet'
    end

    def pre_sentence
      if @sentence.tokens.size >= 2 and
          list_counter?  @sentence.tokens[0] and
          bullet_marker? @sentence.tokens[1]
        @sentence.tokens[0].features << feature
        @sentence.tokens[1].features << feature
      end
    end

  end
end
