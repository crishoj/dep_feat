
module Annotation
  class LemmaOnly < Base

    def mark_token
      @token.form = @token.lemma
      @token.lemma = nil
    end

  end
end