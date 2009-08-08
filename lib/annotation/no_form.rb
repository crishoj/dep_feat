
module Annotation
  class NoForm < Base

    def mark_token
      @token.form = nil
    end

  end
end
