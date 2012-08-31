cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Meule"
	else
		return "Meule"
	end
end

function getDescription()
	if language == "french" then
		return "Défausse 4 sorts de la pioche d'un adversaire."
	else
		return "Discard 4 spells from opponent's deck."
	end
end

function onCast()
	selectTarget("player", "")
end

function onResolve(player)
	-- In discardDeckSpell, the second argument can be either the spell id to discard (if positive), or the number of spells to discard from the top (if negative)
	discardDeckSpell(player, -4)
end
