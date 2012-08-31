childEffectsCount = 1
childEffectsCost = {{0,0,1,0}}	-- 1 death
childEffectsIcon = {"confuse"}
icon = "confuse"

function getName()
	if language == "french" then
		return "Confusion"
	else
		return "Confuse"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Confusion"
	else
		return "Confuse"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : 1µ. L'unité ciblée n'attaque pas ce tour-ci."
	else
		return "Activation cost: 1µ. Target unit can't attack this turn."
	end
end

