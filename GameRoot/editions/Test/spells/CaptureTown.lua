cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Capturer la ville"
	else
		return "Capture town"
	end
end

function getDescription()
	if language == "french" then
		return "La ville ciblée passe sous votre contrôle."
	else
		return "Target town goes under your control."
	end
end

function onCast()
	selectTarget("town", "inrange")
end

function onResolve(town, caster)
	changeTownOwner(town, caster)
end
