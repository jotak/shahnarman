cost = {0,0,0,4}--10}
icon = "ragnarok"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Ragnar�k"
	else
		return "Ragnar�k"
	end
end

function getDescription()
	if language == "french" then
		return "Le Ragnar�k inflige 7 points de d�g�ts � toutes les unit�s du jeu, except�s les Shahmahs."
	else
		return "Ragnar�k deals 7 damages to all game units, except Shahmahs."
	end
end

function onCast()
end

function onResolve(params)
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			if isShahmah(player, unit) == 0 then
				damageUnit(player, unit, 7)
			end
		end
	end
end
