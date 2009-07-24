
# TODO: For Italian: Determine which token the parenthesis binds to (following token unless it's PUNC)

module Annotation
  class OpenClose1 < OpenClose

    def mark_token
      super
      if closing?
        if @token.next
          @token.next.features << 'after-closing'
        end
      end
    end

  end
end
