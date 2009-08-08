
module Annotation
  class Inanimacy < Animacy

    def feature
      'dead'
    end

    def mark_anim
      # Do nothing
    end

    def mark_dead
      @token.features << feature
    end


  end
end
